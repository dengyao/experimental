#include "ProxyManager.h"
#include <limits>
#include <iostream>
#include <proto/MessageHelper.h>
#include <proto/internal.protocol.pb.h>

namespace dbproxy
{
	// 类型转换
	inline bool ToNativeActionType(internal::QueryDBProxyReq::ActoinType type, dbproxy::ActionType &local_type)
	{
		switch (type)
		{
		case internal::QueryDBProxyReq::kSelect:
			local_type = dbproxy::ActionType::kSelect;
			break;
		case internal::QueryDBProxyReq::kInsert:
			local_type = dbproxy::ActionType::kInsert;
			break;
		case internal::QueryDBProxyReq::kUpdate:
			local_type = dbproxy::ActionType::kUpdate;
			break;
		case internal::QueryDBProxyReq::kDelete:
			local_type = dbproxy::ActionType::kDelete;
			break;
		default:
			return false;
		}
		return true;
	}

	ProxyManager::ProxyManager(eddy::IOServiceThreadManager &threads, dbproxy::ProxyImpl<dbproxy::MySQL> &mysql, unsigned int backlog)
		: threads_(threads)
		, mysql_proxy_(mysql)
		, generator_(backlog)
		, timer_(threads.MainThread()->IOService())
		, wait_handler_(std::bind(&ProxyManager::UpdateHandleResult, this, std::placeholders::_1))
	{
		timer_.async_wait(wait_handler_);
	}

	// 接受处理请求
	void ProxyManager::HandleMessage(eddy::TCPSessionHandler &session, google::protobuf::Message *message)
	{
		internal::QueryDBProxyReq *request = dynamic_cast<internal::QueryDBProxyReq*>(message);
		if (request != nullptr)
		{
			// 为这个任务生成唯一序列号
			uint32_t sequence_native = 0;
			if (generator_.Get(sequence_native))
			{
				dbproxy::ActionType type_native;
				if (ToNativeActionType(request->action(), type_native))
				{
					try
					{
						// 添加新的数据库任务
						internal::QueryDBProxyReq::DatabaseType dbtype = request->dbtype();
						requests_.insert(std::make_pair(sequence_native, SSourceInfo(request->sequence(), session.SessionID())));
						if (dbtype == internal::QueryDBProxyReq::kMySQL)
						{
							mysql_proxy_.Append(sequence_native, type_native, request->dbname().c_str(), request->statement().c_str(), request->statement().size());
						}
						else
						{
							requests_.erase(sequence_native);
							generator_.Put(sequence_native);
							assert(false);
						}
					}
					catch (dbproxy::NotFoundDatabase&)
					{
						// 不存在的目标数据库
						requests_.erase(sequence_native);
						generator_.Put(sequence_native);
						RespondErrorCode(session, request->sequence(), internal::DBProxyErrorRsp::kNotFoundDatabase);
					}
					catch (dbproxy::ResourceInsufficiency&)
					{
						// 服务器资源达到上限
						requests_.erase(sequence_native);
						generator_.Put(sequence_native);
						RespondErrorCode(session, request->sequence(), internal::DBProxyErrorRsp::kResourceInsufficiency);
					}
				}
				else
				{
					// 无效的操作类型
					generator_.Put(sequence_native);
					RespondErrorCode(session, request->sequence(), internal::DBProxyErrorRsp::kInvalidAction);
				}
			}
			else
			{
				// 生成序列号失败
				RespondErrorCode(session, request->sequence(), internal::DBProxyErrorRsp::kResourceInsufficiency);
			}
		}
		else
		{
			// 无效的协议
			RespondErrorCode(session, 0, internal::DBProxyErrorRsp::kInvalidProtocol);
		}
	}

	// 更新处理结果
	void ProxyManager::UpdateHandleResult(asio::error_code error_code)
	{
		if (error_code)
		{
			std::cerr << error_code.message() << std::endl;
			return;
		}

		assert(completion_list_.empty());
		mysql_proxy_.GetCompletionQueue(completion_list_);
		for (const auto &result : completion_list_)
		{
			auto found = requests_.find(result.GetSequence());
			assert(found != requests_.end());
			if (found != requests_.end())
			{
				RespondHandleResult(found->second.session_id, found->second.sequence, result);
			}
			generator_.Put(result.GetSequence());
			requests_.erase(found);
		}
		completion_list_.clear();

		timer_.async_wait(wait_handler_);
	}

	// 回复错误码
	void ProxyManager::RespondErrorCode(eddy::TCPSessionHandler &session, uint32_t sequence, int error_code)
	{
		internal::DBProxyErrorRsp error;
		error.set_sequence(sequence);
		error.set_error_code(static_cast<internal::DBProxyErrorRsp::ErrorCode>(error_code));

		eddy::NetMessage message;
		PackageMessage(&error, message);
		session.Send(message);
	}

	// 回复处理结果
	void ProxyManager::RespondHandleResult(eddy::TCPSessionID id, uint32_t sequence, const dbproxy::Result &result)
	{
		eddy::SessionHandlePointer session = threads_.SessionHandler(id);
		if (session != nullptr)
		{
			eddy::NetMessage message;
			if (result.GetErrorCode())
			{
				internal::DBErrorRsp error;
				error.set_sequence(sequence);
				error.set_error_code(result.GetErrorCode().Value());
				error.set_what(result.GetErrorCode().Message());
				PackageMessage(&error, message);
			}
			else
			{
				internal::QueryDBProxyRsp response;
				response.set_sequence(sequence);
				response.set_result(result.GetResult().data(), result.GetResult().size());
				PackageMessage(&response, message);
			}
			session->Send(message);
		}
	}
}
#include "AgentManager.h"
#include <limits>
#include <iostream>
#include <ProtobufCodec.h>
#include <proto/internal.pb.h>
#include "SessionHandle.h"
#include "StatisticalTools.h"

// 类型转换
inline bool ToNativeActionType(internal::QueryDBAgentReq::ActoinType type, ActionType &local_type)
{
	switch (type)
	{
	case internal::QueryDBAgentReq::kSelect:
		local_type = ActionType::kSelect;
		break;
	case internal::QueryDBAgentReq::kInsert:
		local_type = ActionType::kInsert;
		break;
	case internal::QueryDBAgentReq::kUpdate:
		local_type = ActionType::kUpdate;
		break;
	case internal::QueryDBAgentReq::kDelete:
		local_type = ActionType::kDelete;
		break;
	default:
		return false;
	}
	return true;
}

AgentManager::AgentManager(network::IOServiceThreadManager &threads, AgentImpl<MySQL> &mysql, unsigned int backlog)
	: threads_(threads)
	, mysql_agent_(mysql)
	, generator_(backlog)
	, timer_(threads.MainThread()->IOService())
	, statisical_timer_(threads.MainThread()->IOService(), std::chrono::seconds(1))
	, wait_handler_(std::bind(&AgentManager::UpdateHandleResult, this, std::placeholders::_1))
	, statisical_wait_handler_(std::bind(&AgentManager::UpdateStatisicalData, this, std::placeholders::_1))
{
	timer_.async_wait(wait_handler_);
	statisical_timer_.async_wait(statisical_wait_handler_);
}

// 更新处理结果
void AgentManager::UpdateHandleResult(asio::error_code error_code)
{
	if (error_code)
	{
		std::cerr << error_code.message() << std::endl;
		return;
	}

	assert(completion_list_.empty());
	network::NetMessage buffer;
	mysql_agent_.GetCompletionQueue(completion_list_);
	for (const auto &result : completion_list_)
	{
		buffer.Clear();
		auto found = requests_.find(result.GetSequence());
		assert(found != requests_.end());
		if (found != requests_.end())
		{
			switch (found->second.type)
			{
			case ActionType::kSelect:
				StatisticalTools::GetInstance()->AccumulateHandleSelectCount(1);
				break;
			case ActionType::kInsert:
				StatisticalTools::GetInstance()->AccumulateHandleInsertCount(1);
				break;
			case ActionType::kUpdate:
				StatisticalTools::GetInstance()->AccumulateHandleUpdateCount(1);
				break;
			case ActionType::kDelete:
				StatisticalTools::GetInstance()->AccumulateHandleDeleteCount(1);
				break;
			default:
				break;
			}
			RespondHandleResult(found->second.session_id, found->second.sequence, result, buffer);
		}
		generator_.Put(result.GetSequence());
		requests_.erase(found);
	}
	completion_list_.clear();
	timer_.async_wait(wait_handler_);
}

// 更新统计数据
void AgentManager::UpdateStatisicalData(asio::error_code error_code)
{
	if (error_code)
	{
		std::cerr << error_code.message() << std::endl;
		return;
	}
	StatisticalTools::GetInstance()->Flush();
	statisical_timer_.expires_from_now(std::chrono::seconds(1));
	statisical_timer_.async_wait(statisical_wait_handler_);
}

// 接受处理请求
void AgentManager::HandleMessage(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	if (dynamic_cast<internal::QueryDBAgentReq*>(message) != nullptr)
	{
		OnHandleDatabase(session, message, buffer);
	}
	else if (dynamic_cast<internal::DBAgentInfoReq*>(message) != nullptr)
	{
		OnQueryAgentInfo(session, message, buffer);
	}
	else
	{
		RespondErrorCode(session, 0, internal::kInvalidProtocol, buffer);
	}	
}

// 查询服务器信息
void AgentManager::OnQueryAgentInfo(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	buffer.Clear();
	internal::DBAgentInfoRsp response;
	response.set_queue_num(requests_.size());
	response.set_client_num(threads_.SessionNumber());
	response.set_up_volume(StatisticalTools::GetInstance()->UpVolume());
	response.set_down_volume(StatisticalTools::GetInstance()->DownVolume());
	response.set_handle_select_count(StatisticalTools::GetInstance()->HandleSelectCount());
	response.set_handle_insert_count(StatisticalTools::GetInstance()->HandleInsertCount());
	response.set_handle_update_count(StatisticalTools::GetInstance()->HandleUpdateCount());
	response.set_handle_delete_count(StatisticalTools::GetInstance()->HandleDeleteCount());
	ProtubufCodec::Encode(&response, buffer);
	session.Respond(buffer);
}

// 操作数据库
void AgentManager::OnHandleDatabase(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	uint32_t sequence_native = 0;
	internal::QueryDBAgentReq *request = static_cast<internal::QueryDBAgentReq*>(message);
	if (generator_.Get(sequence_native))
	{
		ActionType type_native;
		if (ToNativeActionType(request->action(), type_native))
		{
			try
			{
				// 添加新的数据库任务
				internal::QueryDBAgentReq::DatabaseType dbtype = request->dbtype();
				requests_.insert(std::make_pair(sequence_native, SSourceInfo(type_native, request->sequence(), session.SessionID())));
				if (dbtype == internal::QueryDBAgentReq::kMySQL)
				{
					mysql_agent_.Append(sequence_native, type_native, request->dbname().c_str(), request->statement().c_str(), request->statement().size());
				}
				else
				{
					requests_.erase(sequence_native);
					generator_.Put(sequence_native);
					assert(false);
				}
			}
			catch (NotFoundDatabase &)
			{
				// 不存在的目标数据库
				requests_.erase(sequence_native);
				generator_.Put(sequence_native);
				RespondErrorCode(session, request->sequence(), internal::kNotFoundDatabase, buffer);
			}
			catch (ResourceInsufficiency &)
			{
				// 服务器资源达到上限
				requests_.erase(sequence_native);
				generator_.Put(sequence_native);
				RespondErrorCode(session, request->sequence(), internal::kResourceInsufficiency, buffer);
			}
		}
		else
		{
			// 无效的操作类型
			generator_.Put(sequence_native);
			RespondErrorCode(session, request->sequence(), internal::kInvalidOperation, buffer);
		}
	}
	else
	{
		// 生成序列号失败
		RespondErrorCode(session, request->sequence(), internal::kResourceInsufficiency, buffer);
	}
}

// 回复错误码
void AgentManager::RespondErrorCode(SessionHandle &session, uint32_t sequence, int error_code, network::NetMessage &buffer)
{
	buffer.Clear();
	internal::DBAgentErrorRsp response;
	response.set_sequence(sequence);
	response.set_error_code(static_cast<internal::ErrorCode>(error_code));
	ProtubufCodec::Encode(&response, buffer);
	session.Respond(buffer);
}

// 回复处理结果
void AgentManager::RespondHandleResult(network::TCPSessionID id, uint32_t sequence, const Result &result, network::NetMessage &buffer)
{
	network::SessionHandlePointer session = threads_.SessionHandler(id);
	if (session != nullptr)
	{
		if (result.GetErrorCode())
		{
			internal::DBErrorRsp response;
			response.set_sequence(sequence);
			response.set_error_code(result.GetErrorCode().Value());
			response.set_what(result.GetErrorCode().Message());
			ProtubufCodec::Encode(&response, buffer);
		}
		else
		{
			internal::QueryDBAgentRsp response;
			response.set_sequence(sequence);
			response.set_result(result.GetResult().data(), result.GetResult().size());
			ProtubufCodec::Encode(&response, buffer);
		}
		static_cast<SessionHandle*>(session.get())->Respond(buffer);
	}
}
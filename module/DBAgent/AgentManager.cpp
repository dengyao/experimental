#include "AgentManager.h"
#include <limits>
#include <ProtobufCodec.h>
#include <proto/public_struct.pb.h>
#include <proto/server_internal.pb.h>
#include "Logging.h"
#include "SessionHandle.h"
#include "StatisticalTools.h"

using namespace std::placeholders;

// 类型转换
inline bool ToLocalActionType(svr::QueryDBAgentReq::ActoinType type, ActionType &local_type)
{
	switch (type)
	{
	case svr::QueryDBAgentReq::kCall:
		local_type = ActionType::kCall;
		break;
	case svr::QueryDBAgentReq::kSelect:
		local_type = ActionType::kSelect;
		break;
	case svr::QueryDBAgentReq::kInsert:
		local_type = ActionType::kInsert;
		break;
	case svr::QueryDBAgentReq::kUpdate:
		local_type = ActionType::kUpdate;
		break;
	case svr::QueryDBAgentReq::kDelete:
		local_type = ActionType::kDelete;
		break;
	default:
		return false;
	}
	return true;
}

AgentManager::AgentManager(network::IOServiceThreadManager &threads, AgentMySQL &mysql, unsigned int backlog)
	: threads_(threads)
	, mysql_agent_(mysql)
	, generator_(backlog)
	, timer_(threads.MainThread()->IOService())
	, statisical_timer_(threads.MainThread()->IOService(), std::chrono::seconds(1))
	, wait_handler_(std::bind(&AgentManager::UpdateHandleResult, this, std::placeholders::_1))
	, statisical_wait_handler_(std::bind(&AgentManager::UpdateStatisicalData, this, std::placeholders::_1))
	, dispatcher_(std::bind(&AgentManager::OnUnknownMessage, this, _1, _2, _3))
{
	dispatcher_.RegisterMessageCallback<svr::DBAgentInfoReq>(
		std::bind(&AgentManager::OnQueryAgentInfo, this, _1, _2, _3));
	dispatcher_.RegisterMessageCallback<svr::QueryDBAgentReq>(
		std::bind(&AgentManager::OnHandleDatabase, this, _1, _2, _3));

	timer_.async_wait(wait_handler_);
	statisical_timer_.async_wait(statisical_wait_handler_);
}

// 更新处理结果
void AgentManager::UpdateHandleResult(asio::error_code error_code)
{
	if (error_code)
	{
		logger()->error("{}:{} {}", __FUNCTION__, __LINE__, error_code.message());
		return;
	}

	network::NetMessage buffer;
	assert(completion_lists_.empty());
	mysql_agent_.GetCompletedTask(completion_lists_);
	for (const auto &result : completion_lists_)
	{
		buffer.Clear();
		auto found = requests_.find(result.GetSequence());
		assert(found != requests_.end());
		if (found != requests_.end())
		{
			switch (found->second.type)
			{
			case ActionType::kCall:
				StatisticalTools::GetInstance()->AccumulateHandleCallCount(1);
				break;
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
			SendHandleResult(found->second.session_id, found->second.sequence, result, buffer);
		}
		generator_.Put(result.GetSequence());
		requests_.erase(found);
	}
	completion_lists_.clear();
	timer_.async_wait(wait_handler_);
}

// 更新统计数据
void AgentManager::UpdateStatisicalData(asio::error_code error_code)
{
	if (error_code)
	{
		logger()->error("{}:{} {}", __FUNCTION__, __LINE__, error_code.message());
		return;
	}

	StatisticalTools::GetInstance()->Flush();
	logger()->info("服务器信息：连接数{}，队列数量{}，上行流量{}，下行流量{}，调用次数{}，查询次数{}，插入次数{}，更新次数{}，删除次数{}",
		threads_.SessionNumber(),
		requests_.size(),
		StatisticalTools::GetInstance()->UpVolume(),
		StatisticalTools::GetInstance()->DownVolume(),
		StatisticalTools::GetInstance()->HandleCallCount(),
		StatisticalTools::GetInstance()->HandleSelectCount(),
		StatisticalTools::GetInstance()->HandleInsertCount(),
		StatisticalTools::GetInstance()->HandleUpdateCount(),
		StatisticalTools::GetInstance()->HandleDeleteCount());

	statisical_timer_.expires_from_now(std::chrono::seconds(1));
	statisical_timer_.async_wait(statisical_wait_handler_);
}

// 回复错误码
void AgentManager::SendErrorCode(SessionHandle *session, uint32_t sequence, int error_code, network::NetMessage &buffer)
{
	buffer.Clear();
	svr::DBAgentErrorRsp response;
	response.set_sequence(sequence);
	response.set_error_code(static_cast<pub::ErrorCode>(error_code));
	ProtubufCodec::Encode(&response, buffer);
	session->Write(buffer);
}

// 回复处理结果
void AgentManager::SendHandleResult(network::TCPSessionID id, uint32_t sequence, const HandleResult &result, network::NetMessage &buffer)
{
	network::SessionHandlePointer session = threads_.SessionHandler(id);
	if (session != nullptr)
	{
		if (result.GetErrorCode())
		{
			svr::DBErrorRsp response;
			response.set_sequence(sequence);
			response.set_error_code(result.GetErrorCode().Value());
			response.set_what(result.GetErrorCode().Message());
			ProtubufCodec::Encode(&response, buffer);
		}
		else
		{
			svr::QueryDBAgentRsp response;
			response.set_sequence(sequence);
			response.set_result(result.GetHandleResult().data(), result.GetHandleResult().size());
			ProtubufCodec::Encode(&response, buffer);
		}
		static_cast<SessionHandle*>(session.get())->Write(buffer);
	}
}

// 接收消息
bool AgentManager::OnMessage(SessionHandle *session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	return dispatcher_.OnProtobufMessage(session, message, buffer);
}

// 未定义消息
bool AgentManager::OnUnknownMessage(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	SendErrorCode(static_cast<SessionHandle*>(session), 0, pub::kInvalidProtocol, buffer);
	logger()->warn("协议无效，来自{}:{}", session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
	return true;
}

// 查询服务器信息
bool AgentManager::OnQueryAgentInfo(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	buffer.Clear();
	svr::DBAgentInfoRsp response;
	response.set_queue_num(requests_.size());
	response.set_client_num(threads_.SessionNumber());
	response.set_up_volume(StatisticalTools::GetInstance()->UpVolume());
	response.set_down_volume(StatisticalTools::GetInstance()->DownVolume());
	response.set_handle_call_count(StatisticalTools::GetInstance()->HandleCallCount());
	response.set_handle_select_count(StatisticalTools::GetInstance()->HandleSelectCount());
	response.set_handle_insert_count(StatisticalTools::GetInstance()->HandleInsertCount());
	response.set_handle_update_count(StatisticalTools::GetInstance()->HandleUpdateCount());
	response.set_handle_delete_count(StatisticalTools::GetInstance()->HandleDeleteCount());
	ProtubufCodec::Encode(&response, buffer);
	static_cast<SessionHandle*>(session)->Write(buffer);
	return true;
}

// 操作数据库
bool AgentManager::OnHandleDatabase(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	uint32_t sequence_native = 0;
	svr::QueryDBAgentReq *request = static_cast<svr::QueryDBAgentReq*>(message);
	if (generator_.Get(sequence_native))
	{
		ActionType type_native;
		if (ToLocalActionType(request->action(), type_native))
		{
			try
			{
				// 添加新的数据库任务
				requests_.insert(std::make_pair(sequence_native, SSourceInfo(type_native, request->sequence(), session->SessionID())));
				mysql_agent_.AppendTask(sequence_native, type_native, request->dbname(), std::move(const_cast<std::string&>(request->statement())));
			}
			catch (NotFoundDatabase &)
			{
				// 不存在的目标数据库
				requests_.erase(sequence_native);
				generator_.Put(sequence_native);
				SendErrorCode(static_cast<SessionHandle*>(session), request->sequence(), pub::kNotFoundDatabase, buffer);
				logger()->warn("协议无效，来自{}:{}", session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
			}
			catch (ResourceInsufficiency &)
			{
				// 服务器资源达到上限
				requests_.erase(sequence_native);
				generator_.Put(sequence_native);
				SendErrorCode(static_cast<SessionHandle*>(session), request->sequence(), pub::kResourceInsufficiency, buffer);
				logger()->warn("数据库任务队列已满!");
			}
		}
		else
		{
			// 无效的操作类型
			generator_.Put(sequence_native);
			SendErrorCode(static_cast<SessionHandle*>(session), request->sequence(), pub::kInvalidOperation, buffer);
			logger()->warn("无效的数据库操作，类型{}，来自{}:{}", request->action(), session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
		}
	}
	else
	{
		// 生成序列号失败
		SendErrorCode(static_cast<SessionHandle*>(session), request->sequence(), pub::kResourceInsufficiency, buffer);
		logger()->warn("数据库任务队列已满!");
	}
	return true;
}
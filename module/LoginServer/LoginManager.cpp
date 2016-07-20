#include "LoginManager.h"
#include <DBClient.h>
#include <DBResult.h>
#include <ProtobufCodec.h>
#include <proto/client_login.pb.h>
#include <proto/public_struct.pb.h>
#include <proto/server_internal.pb.h>
#include <common/StringHelper.h>
#include "Logging.h"
#include "ServerConfig.h"
#include "SessionHandle.h"

LoginManager::LoginManager(network::IOServiceThreadManager &threads, const std::vector<SPartition> &partition)
	: threads_(threads)
	, partition_lists_(partition)
{
}

// 回复错误码
void LoginManager::RespondErrorCode(SessionHandle &session, network::NetMessage &buffer, int error_code, const char *what)
{
	buffer.Clear();
	pub::ErrorRsp response;
	response.set_error_code(static_cast<pub::ErrorCode>(error_code));
	if (what != nullptr)
	{
		response.set_what(what);
	}
	ProtubufCodec::Encode(&response, buffer);
	session.Send(buffer);
}

// 处理用户消息
bool LoginManager::HandleUserMessage(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	return true;
}

// 处理用户离线
void LoginManager::HandleUserOffline(SessionHandle &session)
{
	
}

// 处理Linker消息
bool LoginManager::HandleLinkerMessage(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	if (dynamic_cast<svr::LinkerLoginReq*>(message) != nullptr)
	{
		return OnLinkerLogin(session, message, buffer);
	}
	else if (dynamic_cast<svr::UpdateLinkerCapacityReq*>(message) != nullptr)
	{
		return OnLinkerUpdateLoadCapacity(session, message, buffer);
	}
	else
	{
		RespondErrorCode(session, buffer, pub::kInvalidProtocol, message->GetTypeName().c_str());
		logger()->warn("Linker协议无效，来自{}:{}", session.RemoteEndpoint().address().to_string(), session.RemoteEndpoint().port());
		return false;
	}
}

// 处理Linker下线
void LoginManager::HandleLinkerOffline(SessionHandle &session)
{
	for (auto &partition : partition_map_)
	{
		for (auto &item : partition.second.session_map)
		{
			if (item.second.session_id == session.SessionID())
			{
				const uint16_t linker_id = item.first;
				const uint16_t partition_id = partition.first;
				partition.second.session_map.erase(linker_id);
				if (partition.second.session_map.empty())
				{
					partition_map_.erase(partition_id);
				}

				logger()->info("Partition[{}] Linker[{}]下线，来自{}:{}", partition_id, linker_id,
					session.RemoteEndpoint().address().to_string(), session.RemoteEndpoint().port());
				break;
			}
		}
	}
}

// Linker登录
bool LoginManager::OnLinkerLogin(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	auto request = static_cast<svr::LinkerLoginReq*>(message);

	auto partition_iter = partition_map_.find(request->partition_id());
	if (partition_iter != partition_map_.end())
	{
		SLinkerGroup group;
		group.partition_id = request->partition_id();
		partition_map_.insert(std::make_pair(group.partition_id, group));
		partition_iter = partition_map_.find(request->partition_id());
	}

	// 是否重复注册
	auto &session_map = partition_iter->second.session_map;
	for (const auto &item : session_map)
	{
		if (item.second.session_id == session.SessionID())
		{
			RespondErrorCode(session, buffer, pub::kRepeatLogin, message->GetTypeName().c_str());
			logger()->warn("Linker重复注册，来自{}:{}", session.RemoteEndpoint().address().to_string(), session.RemoteEndpoint().port());
			return false;
		}
	}

	// 生成Linker ID
	SLinkerItem linker_item;
	linker_item.port = request->port();
	linker_item.public_ip = request->public_ip();
	linker_item.session_id = session.SessionID();
	linker_item.linker_id = session_map.empty() ? 1 : session_map.rbegin()->first + 1;
	session_map.insert(std::make_pair(linker_item.linker_id, linker_item));

	// 返回结果
	buffer.Clear();
	svr::LinkerLoginRsp response;
	response.set_linker_id(linker_item.linker_id);
	response.set_heartbeat_interval(ServerConfig::GetInstance()->GetHeartbeatInterval());
	logger()->info("Partition[{}] Linker[{}]注册成功，来自{}:{}", request->partition_id(), linker_item.linker_id,
		session.RemoteEndpoint().address().to_string(), session.RemoteEndpoint().port());
	return true;
}

// Linker上报负载量
bool LoginManager::OnLinkerUpdateLoadCapacity(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	auto request = static_cast<svr::UpdateLinkerCapacityReq*>(message);

	uint16_t linker_id = 0;
	uint16_t partition_id = 0;
	for (auto &partition : partition_map_)
	{
		for (auto &item : partition.second.session_map)
		{
			if (item.second.session_id == session.SessionID())
			{
				linker_id = item.first;
				partition_id = partition.first;
				item.second.online_number = request->load();
				break;
			}
		}
	}

	if (linker_id != 0 && partition_id != 0)
	{
		logger()->info("Partition[{}] Linker[{}]上报在线人数{}，来自{}:{}", partition_id, linker_id, request->load(),
			session.RemoteEndpoint().address().to_string(), session.RemoteEndpoint().port());
	}

	return true;
}

// 用户登录
void LoginManager::OnUserSignIn(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{

}

// 用户注册
void LoginManager::OnUserSignUp(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	auto request = static_cast<login::SignUpReq*>(message);

	db::DBClient *db_client = nullptr;
	std::string msg_name = message->GetTypeName();
	network::TCPSessionID session_id = session.SessionID();
	std::string sql = string_helper::format("CALL sign_up('%s', '%s', '%s', '%s', '%s', '%s', '%s', @user_id); SELECT @user_id;",
		request->user().c_str(),
		request->passwd().c_str(),
		session.RemoteEndpoint().address().to_string().c_str(),
		request->has_platform() ? request->platform().c_str() : "",
		request->has_os() ? request->os().c_str() : "",
		request->has_model() ? request->model().c_str() : "",
		request->has_deviceid() ? request->deviceid().c_str() : "");

	db_client->AsyncSelect(db::kMySQL, "db_verify", sql.c_str(), [=](google::protobuf::Message *message)
	{
		network::SessionHandlePointer session_ptr = threads_.SessionHandler(session_id);
		if (session_ptr != nullptr)
		{
			if (dynamic_cast<svr::QueryDBAgentRsp*>(message) != nullptr)
			{
				auto rsp = static_cast<svr::QueryDBAgentRsp*>(message);
				db::WrapResultSet result_set(rsp->result().data(), rsp->result().size());
				db::WrapResultItem item = result_set.GetRow();
				uint32_t user_id = static_cast<uint32_t>(atoll(item[0]));
				if (user_id > 0)
				{

				}
				else
				{
					// 创建账号失败
					pub::ErrorRsp response;
					network::NetMessage message;
					response.set_error_code(pub::kCreateAccountFailed);
					response.set_what(msg_name.c_str());
					ProtubufCodec::Encode(&response, message);
					session_ptr->Send(message);
				}
			}
			else
			{
				// 操作数据库错误
				pub::ErrorRsp response;
				network::NetMessage message;
				response.set_error_code(pub::kDatabaseError);
				response.set_what(msg_name.c_str());
				ProtubufCodec::Encode(&response, message);
				session_ptr->Send(message);
			}
		}
	});
}

// 查询分区
void LoginManager::OnUserQueryPartition(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
}

// 进入分区
void LoginManager::OnUserEntryPartition(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
}
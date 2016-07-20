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
	for (const auto &partition : partition_lists_)
	{
		assert(partition.status >= login::QueryPartitionRsp::StateType_MIN &&
			partition.status <= login::QueryPartitionRsp::StateType_MAX);
		if (partition.status < login::QueryPartitionRsp::StateType_MIN ||
			partition.status > login::QueryPartitionRsp::StateType_MAX)
		{
			logger()->critical("非法的分区状态码!");
			exit(-1);
		}
	}
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

// 处理接受新连接
void LoginManager::HandleAcceptConnection(SessionHandle &session)
{
	connection_map_.insert(std::make_pair(session.SessionID(), SConnection()));
}

// 处理用户消息
bool LoginManager::HandleUserMessage(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	if (dynamic_cast<login::QueryPartitionReq*>(message) != nullptr)
	{
		OnUserQueryPartition(session, message, buffer);
	}
	else if (dynamic_cast<login::SignInReq*>(message) != nullptr)
	{
		OnUserSignIn(session, message, buffer);
	}
	else if (dynamic_cast<login::EntryPartitionReq*>(message) != nullptr)
	{
		OnUserEntryPartition(session, message, buffer);
	}
	else if (dynamic_cast<login::SignUpReq*>(message) != nullptr)
	{
		OnUserSignUp(session, message, buffer);
	}
	else
	{
		RespondErrorCode(session, buffer, pub::kInvalidProtocol, message->GetTypeName().c_str());
		logger()->warn("协议无效，来自{}:{}", session.RemoteEndpoint().address().to_string(), session.RemoteEndpoint().port());
	}
	return true;
}

// 处理用户离线
void LoginManager::HandleUserOffline(SessionHandle &session)
{
	connection_map_.erase(session.SessionID());
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
		for (auto &item : partition.second.linker_map)
		{
			if (item.second.session_id == session.SessionID())
			{
				const uint16_t linker_id = item.first;
				const uint16_t partition_id = partition.first;
				partition.second.linker_map.erase(linker_id);
				if (partition.second.linker_map.empty())
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

	// 分区是否存在
	auto found = std::find_if(partition_lists_.begin(), partition_lists_.end(), [=](const SPartition &partition)
	{
		return partition.id == request->partition_id();
	});

	if (found == partition_lists_.end())
	{
		RespondErrorCode(session, buffer, pub::kRepeatLogin, message->GetTypeName().c_str());
		logger()->warn("分区不存在，Linker登录失败，来自{}:{}", session.RemoteEndpoint().address().to_string(), session.RemoteEndpoint().port());
		return false;
	}

	// 分区是否停机维护
	if (found->status != login::QueryPartitionRsp::kNormal)
	{
		RespondErrorCode(session, buffer, pub::kRepeatLogin, message->GetTypeName().c_str());
		logger()->warn("分区停机维护中，Linker登录失败，来自{}:{}", session.RemoteEndpoint().address().to_string(), session.RemoteEndpoint().port());
		return false;
	}

	auto partition_iter = partition_map_.find(request->partition_id());
	if (partition_iter != partition_map_.end())
	{
		SLinkerGroup group;
		group.partition_id = request->partition_id();
		partition_map_.insert(std::make_pair(group.partition_id, group));
		partition_iter = partition_map_.find(request->partition_id());
	}

	// 是否重复登录
	auto &linker_map = partition_iter->second.linker_map;
	for (const auto &item : linker_map)
	{
		if (item.second.session_id == session.SessionID())
		{
			RespondErrorCode(session, buffer, pub::kRepeatLogin, message->GetTypeName().c_str());
			logger()->warn("Linker重复登录，来自{}:{}", session.RemoteEndpoint().address().to_string(), session.RemoteEndpoint().port());
			return false;
		}
	}

	// 生成Linker ID
	SLinkerItem linker_item;
	linker_item.port = request->port();
	linker_item.public_ip = request->public_ip();
	linker_item.session_id = session.SessionID();
	linker_item.linker_id = linker_map.empty() ? 1 : linker_map.rbegin()->first + 1;
	linker_map.insert(std::make_pair(linker_item.linker_id, linker_item));

	// 返回结果
	buffer.Clear();
	svr::LinkerLoginRsp response;
	response.set_linker_id(linker_item.linker_id);
	response.set_heartbeat_interval(ServerConfig::GetInstance()->GetHeartbeatInterval());
	ProtubufCodec::Encode(&response, buffer);
	session.Send(buffer);

	logger()->info("Partition[{}] Linker[{}]登录成功，来自{}:{}", request->partition_id(), linker_item.linker_id,
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
		for (auto &item : partition.second.linker_map)
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

// 查询分区
void LoginManager::OnUserQueryPartition(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	buffer.Clear();
	login::QueryPartitionRsp response;
	for (const auto &partition : partition_lists_)
	{
		// 计算分区Linker数量
		uint32_t linker_sum = 0;
		auto found = partition_map_.find(partition.id);
		if (found != partition_map_.end())
		{
			linker_sum = found->second.linker_map.size();
		}

		// 添加分区信息
		auto status = static_cast<login::QueryPartitionRsp::StateType>(partition.status);
		login::QueryPartitionRsp::Partition *data = response.add_lists();
		data->set_id(partition.id);
		data->set_name(partition.name);
		data->set_status(linker_sum > 0 ? status : login::QueryPartitionRsp::kShutdown);
		data->set_is_recommend(partition.is_recommend);
	}
	ProtubufCodec::Encode(&response, buffer);
	session.Send(buffer);
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

// 用户登录
void LoginManager::OnUserSignIn(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	auto request = static_cast<login::SignInReq*>(message);
}

// 进入分区
void LoginManager::OnUserEntryPartition(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
}
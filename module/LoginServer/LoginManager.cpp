#include "LoginManager.h"
#include <DBClient.h>
#include <DBResult.h>
#include <ProtobufCodec.h>
#include <proto/client_login.pb.h>
#include <proto/public_struct.pb.h>
#include <proto/server_internal.pb.h>
#include <common/StringHelper.h>
#include "Logging.h"
#include "GlobalObject.h"
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
		auto &linker_lists = partition.second.linker_lists;
		for (auto iter = linker_lists.begin(); iter != linker_lists.end(); ++iter)
		{	
			if (iter->session_id == session.SessionID())
			{
				const uint16_t linker_id = iter->session_id;
				const uint16_t partition_id = partition.first;
				linker_lists.erase(iter);
				if (linker_lists.empty())
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
	auto &linker_lists = partition_iter->second.linker_lists;
	for (const auto &item : linker_lists)
	{
		if (item.session_id == session.SessionID())
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
	if (linker_lists.empty())
	{
		linker_item.linker_id = 1;
	}
	else
	{
		auto max_element = std::max_element(linker_lists.begin(), linker_lists.end(), [](const SLinkerItem &a, const SLinkerItem &b)
		{
			return a.linker_id > b.linker_id;
		});
		linker_item.linker_id = max_element->linker_id + 1;
	}
	linker_lists.push_back(linker_item);

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
		for (auto &item : partition.second.linker_lists)
		{
			if (item.session_id == session.SessionID())
			{
				linker_id = item.linker_id;
				partition_id = partition.first;
				item.online_number = request->load();
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
			linker_sum = found->second.linker_lists.size();
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

	// 拷贝数据
	std::string msg_name = message->GetTypeName();
	network::TCPSessionID session_id = session.SessionID();

	// 构造sql语句
	std::string sql = string_helper::format("CALL sign_up('%s', '%s', '%s', '%s', '%s', '%s', '%s', @user_id); SELECT @user_id;",
		request->user().c_str(),
		request->passwd().c_str(),
		session.RemoteEndpoint().address().to_string().c_str(),
		request->has_platform() ? request->platform().c_str() : "",
		request->has_os() ? request->os().c_str() : "",
		request->has_model() ? request->model().c_str() : "",
		request->has_deviceid() ? request->deviceid().c_str() : "");

	// 执行异步查询
	GlobalDBClient()->AsyncSelect(db::kMySQL, "db_verify", sql.c_str(), [=](google::protobuf::Message *db_message)
	{
		network::SessionHandlePointer session_ptr = threads_.SessionHandler(session_id);
		if (session_ptr != nullptr)
		{
			network::NetMessage new_buffer;
			if (dynamic_cast<svr::QueryDBAgentRsp*>(db_message) != nullptr)
			{			
				auto result = static_cast<svr::QueryDBAgentRsp*>(db_message);
				db::WrapResultSet result_set(result->result().data(), result->result().size());
				db::WrapResultItem item = result_set.GetRow();
				uint32_t user_id = static_cast<uint32_t>(atoll(item[0]));
				if (user_id > 0)
				{
					// 创建账号成功
					login::SignUpRsp response;
					response.set_id(user_id);
					ProtubufCodec::Encode(&response, new_buffer);
					session_ptr->Send(buffer);
					logger()->info("创建账号成功，来自{}:{}", session_ptr->RemoteEndpoint().address().to_string(), session_ptr->RemoteEndpoint().port());
				}
				else
				{
					// 创建账号失败			
					RespondErrorCode(*static_cast<SessionHandle*>(session_ptr.get()), new_buffer, pub::kCreateAccountFailed, msg_name.c_str());
					logger()->info("创建账号失败，来自{}:{}", session_ptr->RemoteEndpoint().address().to_string(), session_ptr->RemoteEndpoint().port());
				}
			}
			else
			{
				// 操作数据库错误
				RespondErrorCode(*static_cast<SessionHandle*>(session_ptr.get()), new_buffer, pub::kDatabaseError, msg_name.c_str());
				logger()->info("创建账号失败，来自{}:{}", session_ptr->RemoteEndpoint().address().to_string(), session_ptr->RemoteEndpoint().port());
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
	auto request = static_cast<login::EntryPartitionReq*>(message);

	// 是否已经验证
	auto found =  connection_map_.find(session.SessionID());
	if (found == connection_map_.end() || found->second.user_id == 0)
	{
		RespondErrorCode(session, buffer, pub::kNotLoggedIn, request->GetTypeName().c_str());
		logger()->info("进入分区失败，玩家未登录，来自{}:{}", session.RemoteEndpoint().address().to_string(), session.RemoteEndpoint().port());
		return;
	}

	// 分区是否存在
	std::vector<SLinkerItem> *linker_map = nullptr;
	auto partition_found = std::find_if(partition_lists_.begin(), partition_lists_.end(), [=](const SPartition &partition)
	{
		return partition.id == request->id();
	});

	if (partition_found != partition_lists_.end() || partition_found->status != login::QueryPartitionRsp::kShutdown)
	{
		auto partition_iter = partition_map_.find(request->id());
		if (partition_iter != partition_map_.end())
		{
			linker_map = &partition_iter->second.linker_lists;
		}
	}

	if (linker_map == nullptr)
	{
		RespondErrorCode(session, buffer, pub::kPartitionNotExist, request->GetTypeName().c_str());
		logger()->info("进入分区失败，分区不存在，来自{}:{}", session.RemoteEndpoint().address().to_string(), session.RemoteEndpoint().port());
		return;
	}

	// 选择在线人数最小的Linker
	auto min_element = std::min_element(linker_map->begin(), linker_map->end(), [](const SLinkerItem &a, const SLinkerItem &b)
	{
		return a.online_number < b.online_number;
	});

	if (min_element == linker_map->end())
	{
		RespondErrorCode(session, buffer, pub::kLinkerNotExist, request->GetTypeName().c_str());
		logger()->info("进入分区失败，没有可用的Linker，来自{}:{}", session.RemoteEndpoint().address().to_string(), session.RemoteEndpoint().port());
		return;
	}

	// 更新Linker验证信息
	uint64_t token = generator_(found->second.user_id);
	auto linker_session = threads_.SessionHandler(min_element->session_id);
	if (linker_session == nullptr)
	{
		RespondErrorCode(session, buffer, pub::kLinkerNotExist, request->GetTypeName().c_str());
		logger()->info("进入分区失败，没有可用的Linker，来自{}:{}", session.RemoteEndpoint().address().to_string(), session.RemoteEndpoint().port());
		return;
	}
	buffer.Clear();
	svr::UpdateUserTokenReq update_token;
	update_token.set_user_id(found->second.user_id);
	update_token.set_token(token);
	ProtubufCodec::Encode(&update_token, buffer);
	linker_session->Send(buffer);

	// 返回分区信息
	buffer.Clear();
	login::EntryPartitionRsp response;
	response.set_ip(min_element->public_ip);
	response.set_port(min_element->port);
	response.set_token(token);
	ProtubufCodec::Encode(&response, buffer);
	session.Send(buffer);

	// 进入分区成功关闭连接
	session.Close();
}
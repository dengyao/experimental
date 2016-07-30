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

using namespace std::placeholders;

LoginManager::LoginManager(network::IOServiceThreadManager &threads, const std::vector<SPartition> &partition)
	: threads_(threads)
	, partition_lists_(partition)
	, timer_(threads.MainThread()->IOService(), std::chrono::seconds(1))
	, wait_handler_(std::bind(&LoginManager::OnUpdateTimer, this, _1))
	, dispatcher_(std::bind(&LoginManager::OnUnknownMessage, this, _1, _2, _3))
{
	for (const auto &partition : partition_lists_)
	{
		assert(partition.status >= cli::QueryPartitionRsp::StateType_MIN &&
			partition.status <= cli::QueryPartitionRsp::StateType_MAX);
		if (partition.status < cli::QueryPartitionRsp::StateType_MIN ||
			partition.status > cli::QueryPartitionRsp::StateType_MAX)
		{
			logger()->critical("非法的分区状态码!");
			exit(-1);
		}
	}

	dispatcher_.RegisterMessageCallback<svr::LinkerLoginReq>(
		std::bind(&LoginManager::OnLinkerLogin, this, _1, _2, _3));
	dispatcher_.RegisterMessageCallback<svr::ReportLinkerReq>(
		std::bind(&LoginManager::OnReportLinker, this, _1, _2, _3));
	dispatcher_.RegisterMessageCallback<cli::QueryPartitionReq>(
		std::bind(&LoginManager::OnUserQueryPartition, this, _1, _2, _3));
	dispatcher_.RegisterMessageCallback<cli::SignInReq>(
		std::bind(&LoginManager::OnUserSignIn, this, _1, _2, _3));
	dispatcher_.RegisterMessageCallback<cli::EntryPartitionReq>(
		std::bind(&LoginManager::OnUserEntryPartition, this, _1, _2, _3));
	dispatcher_.RegisterMessageCallback<cli::SignUpReq>(
		std::bind(&LoginManager::OnUserSignUp, this, _1, _2, _3));

	timer_.async_wait(wait_handler_);
}

// 从数据库查询分区信息
void LoginManager::QueryPartitionInfoByDatabase()
{
	auto callback = [this](google::protobuf::Message *message)
	{
		if (dynamic_cast<svr::QueryDBAgentRsp*>(message) != nullptr)
		{
			std::vector<SPartition> partition_lists;
			auto response = static_cast<svr::QueryDBAgentRsp*>(message);
			db::WrapResultSet result_set(response->result().data(), response->result().size());
			for (unsigned int row = 0; row < result_set.NumRows(); ++row)
			{
				SPartition partition;
				db::WrapResultItem item = result_set.GetRow(row);
				partition.id = atoi(item["id"]);
				partition.name = item["name"];
				partition.status = atoi(item["status"]);
				partition.is_recommend = atoi(item["recommend"]) != 0;
				partition.createtime = item["createtime"];
				partition_lists.push_back(partition);
			}

			partition_lists_ = std::move(partition_lists);
			logger()->debug("更新分区信息成功，共有{}个分区", partition_lists_.size());
		}
	};
	GlobalDBClient()->AsyncSelect(db::kMySQL, ServerConfig::GetInstance()->GetVerifyDBName(), "SELECT * FROM `partition`;", callback);
}

// 更新定时器
void LoginManager::OnUpdateTimer(asio::error_code error_code)
{
	if (error_code)
	{
		logger()->error("{}:{} {}", __FUNCTION__, __LINE__, error_code.message());
		return;
	}

	// 关闭超时用户连接
	for (auto iter = user_session_.begin(); iter != user_session_.end();)
	{
		auto duration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - iter->second.time).count();
		if (duration >= ServerConfig::GetInstance()->GetMaxUserOnlineTime())
		{
			auto session_ptr = threads_.SessionHandler(iter->first);
			if (session_ptr != nullptr)
			{
				auto session = static_cast<SessionHandle*>(session_ptr.get());
				if (session->GetRoleType() == RoleType::kUser)
				{
					session->Close();
				}
			}
			iter = user_session_.erase(iter);
		}
		else
		{
			++iter;
		}
	}

	// 更新分区信息
	QueryPartitionInfoByDatabase();

	timer_.expires_from_now(std::chrono::seconds(1));
	timer_.async_wait(wait_handler_);
}

// 回复错误码
void LoginManager::SendErrorCode(SessionHandle *session, network::NetMessage &buffer, int error_code, const char *what)
{
	buffer.Clear();
	pub::ErrorRsp response;
	response.set_error_code(static_cast<pub::ErrorCode>(error_code));
	if (what != nullptr)
	{
		response.set_what(what);
	}
	ProtubufCodec::Encode(&response, buffer);
	session->Send(buffer);
}

// 用户连接
void LoginManager::OnConnect(SessionHandle *session)
{
	user_session_.insert(std::make_pair(session->SessionID(), SUserSession()));
}

// 接收用户消息
bool LoginManager::OnUserMessage(SessionHandle *session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	return dispatcher_.OnProtobufMessage(session, message, buffer);
}

// 用户关闭连接
void LoginManager::OnUserClose(SessionHandle *session)
{
	user_session_.erase(session->SessionID());
}

// 未定义消息
bool LoginManager::OnUnknownMessage(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	SendErrorCode(static_cast<SessionHandle*>(session), buffer, pub::kInvalidProtocol, message->GetTypeName().c_str());
	logger()->warn("协议无效，来自{}:{}", session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
	return true;
}

// 接收Linker消息
bool LoginManager::OnLinkerMessage(SessionHandle *session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	return dispatcher_.OnProtobufMessage(session, message, buffer);
}

// Linker关闭连接
void LoginManager::OnLinkerClose(SessionHandle *session)
{
	for (auto &partition : partition_map_)
	{
		auto &linker_lists = partition.second.linker_lists;
		for (auto iter = linker_lists.begin(); iter != linker_lists.end(); ++iter)
		{	
			if (iter->session_id == session->SessionID())
			{
				const uint16_t linker_id = iter->linker_id;
				const uint16_t partition_id = partition.first;

				linker_lists.erase(iter);
				partition.second.generator.Put(linker_id);	
				if (linker_lists.empty())
				{
					partition_map_.erase(partition_id);
				}

				logger()->info("partition[{}] linker[{}]下线，来自{}:{}", partition_id, linker_id,
					session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
				return;
			}
		}
	}
}

// Linker登录
bool LoginManager::OnLinkerLogin(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	auto request = static_cast<svr::LinkerLoginReq*>(message);

	// 分区是否存在
	auto found = std::find_if(partition_lists_.begin(), partition_lists_.end(), [=](const SPartition &partition)
	{
		return partition.id == request->partition_id();
	});

	if (found == partition_lists_.end())
	{
		SendErrorCode(static_cast<SessionHandle*>(session), buffer, pub::kRepeatLogin, message->GetTypeName().c_str());
		logger()->warn("分区不存在，Linker登录失败，来自{}:{}", session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
		return false;
	}

	// 分区是否停机维护
	if (found->status != cli::QueryPartitionRsp::kNormal)
	{
		SendErrorCode(static_cast<SessionHandle*>(session), buffer, pub::kRepeatLogin, message->GetTypeName().c_str());
		logger()->warn("分区停机维护中，Linker登录失败，来自{}:{}", session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
		return false;
	}

	auto partition_iter = partition_map_.find(request->partition_id());
	if (partition_iter == partition_map_.end())
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
		if (item.session_id == session->SessionID() || (request->has_linker_id() && request->linker_id() == item.linker_id))
		{
			SendErrorCode(static_cast<SessionHandle*>(session), buffer, pub::kRepeatLogin, message->GetTypeName().c_str());
			logger()->warn("Linker重复登录，来自{}:{}", session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
			return false;
		}
	}

	// 生成Linker ID
	SLinkerItem linker_item;
	linker_item.port = request->port();
	linker_item.public_ip = request->public_ip();
	linker_item.session_id = session->SessionID();
	if (request->has_linker_id())
	{
		linker_item.linker_id = request->linker_id();
		partition_iter->second.generator.erase(linker_item.linker_id);
	}
	else
	{
		uint32_t linker_id = 0;
		if (partition_iter->second.generator.Get(linker_id))
		{
			linker_item.linker_id = linker_id;
		}
		else
		{
			logger()->warn("生成LinkerID失败!，来自{}:{}", session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
			return false;
		}
	}
	linker_lists.push_back(linker_item);

	// 返回结果
	buffer.Clear();
	svr::LinkerLoginRsp response;
	response.set_linker_id(linker_item.linker_id);
	response.set_heartbeat_interval(ServerConfig::GetInstance()->GetHeartbeatInterval());
	ProtubufCodec::Encode(&response, buffer);
	session->Send(buffer);

	logger()->info("partition[{}] linker[{}]登录成功，来自{}:{}", request->partition_id(), linker_item.linker_id,
		session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());

	return true;
}

// Linker上报负载量
bool LoginManager::OnReportLinker(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	auto request = static_cast<svr::ReportLinkerReq*>(message);

	uint16_t linker_id = 0;
	uint16_t partition_id = 0;
	for (auto &partition : partition_map_)
	{
		for (auto &item : partition.second.linker_lists)
		{
			if (item.session_id == session->SessionID())
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
		logger()->info("partition[{}] linker[{}]上报在线人数{}，来自{}:{}", partition_id, linker_id, request->load(),
			session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
	}

	return true;
}

// 查询分区
bool LoginManager::OnUserQueryPartition(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	buffer.Clear();
	cli::QueryPartitionRsp response;
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
		auto status = static_cast<cli::QueryPartitionRsp::StateType>(partition.status);
		cli::QueryPartitionRsp::Partition *data = response.add_lists();
		data->set_id(partition.id);
		data->set_name(partition.name);
		data->set_status(linker_sum > 0 ? status : cli::QueryPartitionRsp::kShutdown);
		data->set_is_recommend(partition.is_recommend);
	}
	ProtubufCodec::Encode(&response, buffer);
	session->Send(buffer);
	return true;
}

// 用户注册
bool LoginManager::OnUserSignUp(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	auto request = static_cast<cli::SignUpReq*>(message);

	// 拷贝回调数据
	std::string msg_name = message->GetTypeName();
	network::TCPSessionID session_id = session->SessionID();

	// 构造sql语句
	std::string sql = string_helper::format("CALL sign_up('%s', '%s', '%s', '%s', '%s', '%s', '%s', @user_id);",
		request->user().c_str(),
		request->passwd().c_str(),
		session->RemoteEndpoint().address().to_string().c_str(),
		request->has_platform() ? request->platform().c_str() : "",
		request->has_os() ? request->os().c_str() : "",
		request->has_model() ? request->model().c_str() : "",
		request->has_deviceid() ? request->deviceid().c_str() : "");

	// 执行异步查询
	auto callback = [this, msg_name, session_id](google::protobuf::Message *db_message)
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
				uint32_t user_id = static_cast<uint32_t>(atoll(item["@user_id"]));
				if (user_id > 0)
				{
					// 创建账号成功
					cli::SignUpRsp response;
					response.set_id(user_id);
					ProtubufCodec::Encode(&response, new_buffer);
					session_ptr->Send(new_buffer);
					logger()->info("创建账号成功，来自{}:{}", session_ptr->RemoteEndpoint().address().to_string(), session_ptr->RemoteEndpoint().port());
				}
				else
				{
					// 创建账号失败			
					SendErrorCode(static_cast<SessionHandle*>(session_ptr.get()), new_buffer, pub::kCreateAccountFailed, msg_name.c_str());
					logger()->info("创建账号失败，来自{}:{}", session_ptr->RemoteEndpoint().address().to_string(), session_ptr->RemoteEndpoint().port());
				}
			}
			else
			{
				// 操作数据库错误
				SendErrorCode(static_cast<SessionHandle*>(session_ptr.get()), new_buffer, pub::kDatabaseError, msg_name.c_str());
				logger()->info("数据库错误，创建账号失败，来自{}:{}", session_ptr->RemoteEndpoint().address().to_string(), session_ptr->RemoteEndpoint().port());
			}
		}
	};
	GlobalDBClient()->AsyncCall(db::kMySQL, ServerConfig::GetInstance()->GetVerifyDBName(), sql.c_str(), callback);
	return true;
}

// 用户登录
bool LoginManager::OnUserSignIn(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	auto request = static_cast<cli::SignInReq*>(message);

	// 拷贝回调数据
	std::string msg_name = message->GetTypeName();
	network::TCPSessionID session_id = session->SessionID();

	// 构造sql语句
	std::string sql = string_helper::format("CALL sign_in('%s','%s','%s','%s',@userid);",
		request->user().c_str(),
		request->passwd().c_str(),
		session->RemoteEndpoint().address().to_string().c_str(),
		request->has_deviceid() ? request->deviceid().c_str() : "");

	// 执行异步查询
	auto callback = [this, msg_name, session_id](google::protobuf::Message *db_message)
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
				uint32_t user_id = static_cast<uint32_t>(atoll(item["@userid"]));
				if (user_id > 0)
				{
					// 更新连接
					auto found = user_session_.find(session_id);
					if (found != user_session_.end())
					{
						found->second.user_id = user_id;
					}

					// 登录成功
					cli::SignInRsp response;
					response.set_id(user_id);
					ProtubufCodec::Encode(&response, new_buffer);
					session_ptr->Send(new_buffer);
					logger()->info("用户登录成功，来自{}:{}", session_ptr->RemoteEndpoint().address().to_string(), session_ptr->RemoteEndpoint().port());
				}
				else
				{
					// 登录失败			
					SendErrorCode(static_cast<SessionHandle*>(session_ptr.get()), new_buffer, pub::kUsernameOrPasswordError, msg_name.c_str());
					logger()->info("用户登录失败，来自{}:{}", session_ptr->RemoteEndpoint().address().to_string(), session_ptr->RemoteEndpoint().port());
				}
			}
			else
			{
				// 操作数据库错误
				SendErrorCode(static_cast<SessionHandle*>(session_ptr.get()), new_buffer, pub::kDatabaseError, msg_name.c_str());
				logger()->info("数据库错误，用户登录失败，来自{}:{}", session_ptr->RemoteEndpoint().address().to_string(), session_ptr->RemoteEndpoint().port());
			}
		}
	};
	GlobalDBClient()->AsyncCall(db::kMySQL, ServerConfig::GetInstance()->GetVerifyDBName(), sql.c_str(), callback);
	return true;
}

// 进入分区
bool LoginManager::OnUserEntryPartition(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	auto request = static_cast<cli::EntryPartitionReq*>(message);

	// 是否已经验证
	auto found = user_session_.find(session->SessionID());
	if (found == user_session_.end() || found->second.user_id == 0)
	{
		SendErrorCode(static_cast<SessionHandle*>(session), buffer, pub::kNotLoggedIn, request->GetTypeName().c_str());
		logger()->info("进入分区失败，玩家未登录，来自{}:{}", session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
		return false;
	}

	// 分区是否存在
	std::vector<SLinkerItem> *linker_map = nullptr;
	auto partition_found = std::find_if(partition_lists_.begin(), partition_lists_.end(), [=](const SPartition &partition)
	{
		return partition.id == request->id();
	});

	if (partition_found != partition_lists_.end() || partition_found->status != cli::QueryPartitionRsp::kShutdown)
	{
		auto partition_iter = partition_map_.find(request->id());
		if (partition_iter != partition_map_.end())
		{
			linker_map = &partition_iter->second.linker_lists;
		}
	}

	if (linker_map == nullptr)
	{
		SendErrorCode(static_cast<SessionHandle*>(session), buffer, pub::kPartitionNotExist, request->GetTypeName().c_str());
		logger()->info("进入分区失败，分区不存在，来自{}:{}", session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
		return false;
	}

	// 选择在线人数最小的Linker
	auto min_element = std::min_element(linker_map->begin(), linker_map->end(), [](const SLinkerItem &a, const SLinkerItem &b)
	{
		return a.online_number < b.online_number;
	});

	if (min_element == linker_map->end())
	{
		SendErrorCode(static_cast<SessionHandle*>(session), buffer, pub::kLinkerNotExist, request->GetTypeName().c_str());
		logger()->info("进入分区失败，没有可用的Linker，来自{}:{}", session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
		return false;
	}

	// 更新Linker验证信息
	uint64_t token = generator_(found->second.user_id);
	auto linker_session = threads_.SessionHandler(min_element->session_id);
	if (linker_session == nullptr)
	{
		SendErrorCode(static_cast<SessionHandle*>(session), buffer, pub::kLinkerNotExist, request->GetTypeName().c_str());
		logger()->info("进入分区失败，没有可用的Linker，来自{}:{}", session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
		return false;
	}
	buffer.Clear();
	svr::UpdateTokenReq update_token;
	update_token.set_user_id(found->second.user_id);
	update_token.set_token(token);
	ProtubufCodec::Encode(&update_token, buffer);
	linker_session->Send(buffer);

	// 返回分区信息
	buffer.Clear();
	cli::EntryPartitionRsp response;
	response.set_ip(min_element->public_ip);
	response.set_port(min_element->port);
	response.set_token(token);
	ProtubufCodec::Encode(&response, buffer);
	session->Send(buffer);

	// 进入分区成功关闭连接
	session->Close();
	return true;
}
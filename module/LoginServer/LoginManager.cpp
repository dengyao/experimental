#include "LoginManager.h"
#include <DBClient.h>
#include <DBResult.h>
#include <ProtobufCodec.h>
#include <proto/client_login.pb.h>
#include <proto/public_struct.pb.h>
#include <proto/server_internal.pb.h>
#include <common/StringHelper.h>
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

// 处理Linker消息
bool LoginManager::HandleLinkerMessage(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	if (dynamic_cast<svr::RegisterLinkerReq*>(message) != nullptr)
	{
		return OnLinkerRegister(session, message, buffer);
	}
	else if (dynamic_cast<svr::UpdateLinkerCapacityReq*>(message) != nullptr)
	{
		return OnLinkerUpdateLoadCapacity(session, message, buffer);
	}
	else
	{
		RespondErrorCode(session, buffer, pub::kInvalidProtocol, message->GetTypeName().c_str());
		return false;
	}
}

// Linker注册
bool LoginManager::OnLinkerRegister(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	// Linker是否已注册
	auto request = static_cast<svr::RegisterLinkerReq*>(message);
	if (linker_session_map_.find(request->partition_id()) != linker_session_map_.end())
	{
		return false;
	}

	return true;
}

// Linker上报负载量
bool LoginManager::OnLinkerUpdateLoadCapacity(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
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

	db::DBClient *db_client;
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
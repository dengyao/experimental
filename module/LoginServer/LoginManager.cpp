#include "LoginManager.h"
#include <DBClient.h>
#include <ProtobufCodec.h>
#include <proto/client_login.pb.h>
#include <proto/public_struct.pb.h>
#include <proto/server_internal.pb.h>
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
	return true;
}

// Linker上报负载量
bool LoginManager::OnLinkerUpdateLoadCapacity(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	return true;
}

// 用户登录
void OnUserSignInStep1(network::TCPSessionID id, std::shared_ptr<google::protobuf::Message> parameter, google::protobuf::Message *message)
{

}

// 用户登录
void LoginManager::OnUserSignIn(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
}

// 用户注册
void LoginManager::OnUserSignUp(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
}

// 查询分区
void LoginManager::OnUserQueryPartition(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
}

// 进入分区
void LoginManager::OnUserEntryPartition(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
}
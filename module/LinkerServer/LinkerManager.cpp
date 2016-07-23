#include "LinkerManager.h"
#include <ProtobufCodec.h>
#include <proto/client_link.pb.h>
#include <proto/public_struct.pb.h>
#include <proto/server_internal.pb.h>
#include "Logging.h"
#include "SessionHandle.h"

LinkerManager::LinkerManager(network::IOServiceThreadManager &threads)
	: threads_(threads)
	, timer_(threads.MainThread()->IOService(), std::chrono::seconds(1))
	, wait_handler_(std::bind(&LinkerManager::OnUpdateTimer, this, std::placeholders::_1))
{
}

// 回复错误码
void LinkerManager::RespondErrorCodeToUser(SessionHandle *session, network::NetMessage &buffer, int error_code, const char *what)
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

// 处理用户连接
void LinkerManager::HandleUserConnected(SessionHandle *session)
{
	unauth_user_session_.insert(session->SessionID());
}

// 处理来自用户的消息
void LinkerManager::HandleMessageFromUser(SessionHandle *session, google::protobuf::Message *messsage, network::NetMessage &buffer)
{
	// 用户是否验证
	auto found = reverse_user_session_.find(session->SessionID());
	if (found == reverse_user_session_.end())
	{
		auto request = dynamic_cast<cli::UserAuthReq*>(messsage);
		if (request == nullptr)
		{
			return RespondErrorCodeToUser(session, buffer, pub::kUserUnverified, messsage->GetTypeName().c_str());
		}

		auto auth_iter = user_auth_.find(request->token());
		if (auth_iter == user_auth_.end())
		{		
			return RespondErrorCodeToUser(session, buffer, pub::kAuthenticationFailure, messsage->GetTypeName().c_str());
		}

		cli::UserAuthRsp response;
		respo
	}
}

// 处理用户关闭连接
void LinkerManager::HandleUserClose(SessionHandle *session)
{
	unauth_user_session_.erase(session->SessionID());
	auto found = reverse_user_session_.find(session->SessionID());
	if (found != reverse_user_session_.end())
	{
		user_session_.erase(found->first);
		reverse_user_session_.erase(found);
	}
}

// 处理来自路由的消息
void LinkerManager::HandleMessageFromRouter(router::Connector *connector, google::protobuf::Message *messsage, network::NetMessage &buffer)
{
}

// 处理来自登录服务器的消息
void LinkerManager::HandleMessageFromLoginServer(LoginConnector *connector, google::protobuf::Message *messsage, network::NetMessage &buffer)
{
	auto request = dynamic_cast<svr::UpdateTokenReq*>(messsage);
	if (request != nullptr)
	{
		// 更新Token
		SUserAuth auth;
		auth.user_id = request->user_id();
		user_auth_.insert(std::make_pair(request->token(), auth));
		logger()->info("更新用户Token，{}:{}", request->user_id(), request->token());
	}
	else
	{
		logger()->warn("已忽略来自登录服务器的请求，{}", messsage->GetTypeName());
	}
}

// 更新定时器
void LinkerManager::OnUpdateTimer(asio::error_code error_code)
{
	if (error_code)
	{
		logger()->error("{}:{} {}", __FUNCTION__, __LINE__, error_code.message());
		return;
	}

	// 删除超时Token
	for (auto iter = user_auth_.begin(); iter != user_auth_.end();)
	{
		if (std::chrono::steady_clock::now() - iter->second.time >= std::chrono::seconds(60))
		{
			iter = user_auth_.erase(iter);
		}
		{
			++iter;
		}
	}

	// 关闭长时间未验证的连接
	for (auto iter = unauth_user_session_.begin(); iter != unauth_user_session_.end();)
	{
		auto session = static_cast<SessionHandle*>(threads_.SessionHandler(*iter).get());
		if (session != nullptr && session->IsAuthTimeout())
		{
			session->Close();
			iter = unauth_user_session_.erase(iter);
		}
		else
		{
			++iter;
		}
	}

	timer_.expires_from_now(std::chrono::seconds(1));
	timer_.async_wait(wait_handler_);
}
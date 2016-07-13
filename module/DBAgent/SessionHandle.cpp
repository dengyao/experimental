#include "SessionHandle.h"
#include <proto/MessageHelper.h>
#include <proto/internal.protocol.pb.h>
#include "AgentManager.h"

SessionHandle::SessionHandle(AgentManager &manager)
	: is_logged_(false)
	, agent_manager_(manager)
{
}

void SessionHandle::OnConnect()
{
}

void SessionHandle::OnMessage(network::NetMessage &message)
{
	auto respond = UnpackageMessage(message);
	if (respond == nullptr)
	{
		agent_manager_.RespondErrorCode(*this, 0, internal::kInvalidProtocol);
		return;
	}

	if (!is_logged_)
	{
		// 首先必须登录
		if (dynamic_cast<internal::PingReq*>(respond.get()) == nullptr)
		{
			if (dynamic_cast<internal::LoginDBAgentReq*>(respond.get()) == nullptr)
			{
				agent_manager_.RespondErrorCode(*this, 0, internal::kNotLoggedIn);
			}
			else
			{
				message.Clear();
				is_logged_ = true;
				internal::LoginDBAgentRsp login_rsp;
				login_rsp.set_heartbeat_interval(60);
				PackageMessage(&login_rsp, message);
				Send(message);
			}
		}
	}
	else if (dynamic_cast<internal::PingReq*>(respond.get()) != nullptr)
	{
		// 处理心跳
		message.Clear();
		internal::PongRsp pong;
		PackageMessage(&pong, message);
		Send(message);
	}
	else
	{
		agent_manager_.HandleMessage(*this, respond.get());
	}
}

void SessionHandle::OnClose()
{
}

network::MessageFilterPointer CreateMessageFilter()
{
	return std::make_shared<network::DefaultMessageFilter>();
}

network::SessionHandlePointer CreateSessionHandle(AgentManager &agent_manager)
{
	return std::make_shared<SessionHandle>(agent_manager);
}
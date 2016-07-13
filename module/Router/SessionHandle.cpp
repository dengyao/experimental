#include "SessionHandle.h"
#include "RouterManager.h"
#include <proto/MessageHelper.h>
#include <proto/internal.protocol.pb.h>


SessionHandle::SessionHandle(RouterManager &gw_manager)
	: is_logged_(false)
	, gw_manager_(gw_manager)
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
		gw_manager_.RespondErrorCode(*this, internal::kInvalidProtocol);
		return;
	}

	if (!is_logged_)
	{
		// 首先必须登录
		if (dynamic_cast<internal::PingReq*>(respond.get()) == nullptr)
		{
			if (dynamic_cast<internal::LoginRouterReq*>(respond.get()) == nullptr)
			{
				gw_manager_.RespondErrorCode(*this, internal::kNotLoggedIn);
			}
			else
			{
				message.Clear();
				is_logged_ = true;
				internal::LoginRouterRsp login_rsp;
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
		gw_manager_.HandleMessage(*this, respond.get(), message);
	}
}

void SessionHandle::OnClose()
{
	gw_manager_.HandleServerOffline(*this);
}

network::MessageFilterPointer CreateMessageFilter()
{
	return std::make_shared<network::DefaultMessageFilter>();
}

network::SessionHandlePointer CreateSessionHandle(RouterManager &gw_manager)
{
	return std::make_shared<SessionHandle>(gw_manager);
}
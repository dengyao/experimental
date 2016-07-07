#include "SessionHandle.h"
#include <proto/MessageHelper.h>
#include <proto/internal.protocol.pb.h>
#include "ProxyManager.h"

namespace dbproxy
{
	SessionHandle::SessionHandle(ProxyManager &manager)
		: is_logged_(false)
		, proxy_manager_(manager)
	{
	}

	void SessionHandle::OnConnect()
	{
	}

	void SessionHandle::OnMessage(eddy::NetMessage &message)
	{
		auto request = UnpackageMessage(message);
		if (request == nullptr)
		{
			proxy_manager_.RespondErrorCode(*this, 0, internal::DBProxyErrorRsp::kInvalidProtocol);
		}

		if (!is_logged_)
		{
			// 首先必须登录
			if (dynamic_cast<internal::PingReq*>(request.get()) == nullptr)
			{
				if (dynamic_cast<internal::LoginDBProxyReq*>(request.get()) == nullptr)
				{
					proxy_manager_.RespondErrorCode(*this, 0, internal::DBProxyErrorRsp::kNotLogged);
				}
				else
				{
					is_logged_ = true;
					eddy::NetMessage message;
					internal::LoginDBProxyRsp login_rsp;
					login_rsp.set_heartbeat_interval(60);
					PackageMessage(&login_rsp, message);
					Send(message);
				}
			}
		}
		else if (dynamic_cast<internal::PingReq*>(request.get()))
		{
			// 处理心跳
			internal::PongRsp pong;
			eddy::NetMessage message;
			PackageMessage(&pong, message);
			Send(message);
		}
		else
		{
			proxy_manager_.HandleMessage(*this, request.get());
		}
	}

	void SessionHandle::OnClose()
	{
	}

	eddy::MessageFilterPointer CreateMessageFilter()
	{
		return std::make_shared<eddy::DefaultMessageFilter>();
	}

	eddy::SessionHandlePointer CreateSessionHandle(ProxyManager &proxy_manager)
	{
		return std::make_shared<SessionHandle>(proxy_manager);
	}
}
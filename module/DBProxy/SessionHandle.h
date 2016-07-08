#ifndef __DBPROXY_SESSION_HANDLE_H__
#define __DBPROXY_SESSION_HANDLE_H__

#include <eddyserver.h>

namespace dbproxy
{
	class ProxyManager;

	class SessionHandle : public eddy::TCPSessionHandler
	{
	public:
		SessionHandle(ProxyManager &manager);

	private:
		virtual void OnConnect() override;

		virtual void OnMessage(eddy::NetMessage &message) override;

		virtual void OnClose() override;

	private:
		bool          is_logged_;
		ProxyManager& proxy_manager_;
	};

	eddy::MessageFilterPointer CreateMessageFilter();
	eddy::SessionHandlePointer CreateSessionHandle(ProxyManager &proxy_manager);
}

#endif

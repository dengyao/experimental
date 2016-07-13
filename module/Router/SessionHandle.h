#ifndef __DBPROXY_SESSION_HANDLE_H__
#define __DBPROXY_SESSION_HANDLE_H__

#include <network.h>

class RouterManager;

class SessionHandle : public network::TCPSessionHandler
{
public:
	SessionHandle(RouterManager &gw_manager);

private:
	virtual void OnConnect() override;

	virtual void OnMessage(network::NetMessage &message) override;

	virtual void OnClose() override;

private:
	bool            is_logged_;
	RouterManager& gw_manager_;
};

network::MessageFilterPointer CreateMessageFilter();
network::SessionHandlePointer CreateSessionHandle(RouterManager &gw_manager);

#endif

#ifndef __DBPROXY_SESSION_HANDLE_H__
#define __DBPROXY_SESSION_HANDLE_H__

#include <eddyserver.h>

class GatewayManager;

class SessionHandle : public eddy::TCPSessionHandler
{
public:
	SessionHandle(GatewayManager &gw_manager);

private:
	virtual void OnConnect() override;

	virtual void OnMessage(eddy::NetMessage &message) override;

	virtual void OnClose() override;

private:
	bool            is_logged_;
	GatewayManager& gw_manager_;
};

eddy::MessageFilterPointer CreateMessageFilter();
eddy::SessionHandlePointer CreateSessionHandle(GatewayManager &gw_manager);

#endif

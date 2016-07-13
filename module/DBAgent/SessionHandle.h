#ifndef __DBPROXY_SESSION_HANDLE_H__
#define __DBPROXY_SESSION_HANDLE_H__

#include <network.h>

class AgentManager;

class SessionHandle : public network::TCPSessionHandler
{
public:
	SessionHandle(AgentManager &manager);

private:
	virtual void OnConnect() override;

	virtual void OnMessage(network::NetMessage &message) override;

	virtual void OnClose() override;

private:
	bool          is_logged_;
	AgentManager& agent_manager_;
};

network::MessageFilterPointer CreateMessageFilter();
network::SessionHandlePointer CreateSessionHandle(AgentManager &agent_manager);

#endif

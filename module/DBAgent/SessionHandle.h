#ifndef __DBPROXY_SESSION_HANDLE_H__
#define __DBPROXY_SESSION_HANDLE_H__

#include <eddyserver.h>

class AgentManager;

class SessionHandle : public eddy::TCPSessionHandler
{
public:
	SessionHandle(AgentManager &manager);

private:
	virtual void OnConnect() override;

	virtual void OnMessage(eddy::NetMessage &message) override;

	virtual void OnClose() override;

private:
	bool          is_logged_;
	AgentManager& agent_manager_;
};

eddy::MessageFilterPointer CreateMessageFilter();
eddy::SessionHandlePointer CreateSessionHandle(AgentManager &agent_manager);

#endif

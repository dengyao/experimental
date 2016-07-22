#ifndef __LOGIN_SESSION_HANDLE_H__
#define __LOGIN_SESSION_HANDLE_H__

#include <network.h>

class LoginSessionHandle : public network::TCPSessionHandler
{
	friend class LoginConnector;

public:
	LoginSessionHandle(LoginConnector *connector, std::shared_ptr<bool> &life);

private:
	virtual void OnConnect();

	virtual void OnMessage(network::NetMessage &message);

	virtual void OnClose();

private:
	void HeartbeatCountdown();

	std::shared_ptr<bool>& GetShared();

	void SetHeartbeatInterval(uint32_t interval);

private:
	LoginConnector*       connector_;
	std::shared_ptr<bool> connector_life_;
	uint32_t              counter_;
	uint32_t              heartbeat_interval_;
};

#endif
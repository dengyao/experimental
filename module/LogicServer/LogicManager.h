#ifndef __LOGIC_MANAGER_H__
#define __LOGIC_MANAGER_H__

#include <ProtobufDispatcher.h>

namespace gateway
{
	class GatewayClient;
}

class LogicManager
{
public:
	LogicManager(network::IOServiceThreadManager &threads);

public:
	// 网关消息
	void OnGatewayServerMessage(gateway::GatewayClient *connector, google::protobuf::Message *messsage, network::NetMessage &buffer);

private:
	// 未定义消息
	bool OnUnknownMessage(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer);

private:
	network::IOServiceThreadManager&	threads_;
	ProtobufDispatcher					dispatcher_;
};

#endif

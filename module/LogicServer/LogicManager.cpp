#include "LogicManager.h"
#include <GWClient.h>
#include "Logging.h"
using namespace std::placeholders;

LogicManager::LogicManager(network::IOServiceThreadManager &threads)
	: threads_(threads)
	, dispatcher_(std::bind(&LogicManager::OnUnknownMessage, this, _1, _2, _3))
{
}

// 网关服务器消息
void LogicManager::OnGatewayServerMessage(gateway::GatewayClient *connector, google::protobuf::Message *messsage, network::NetMessage &buffer)
{
}

// 未定义消息
bool LogicManager::OnUnknownMessage(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	return false;
}
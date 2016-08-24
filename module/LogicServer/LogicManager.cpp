#include "LogicManager.h"
#include <GWClient.h>
#include "Logging.h"
using namespace std::placeholders;

LogicManager::LogicManager(network::IOServiceThreadManager &threads)
	: threads_(threads)
	, dispatcher_(std::bind(&LogicManager::OnUnknownMessage, this, _1, _2, _3))
{
}

// 未定义消息
bool LogicManager::OnUnknownMessage(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	return false;
}

// 网关消息
void LogicManager::OnGatewayServerMessage(gw::GWClient *connector, google::protobuf::Message *messsage, network::NetMessage &buffer)
{
	if (!dispatcher_.OnProtobufMessage(connector->ContextSession(), messsage, buffer))
	{
		logger()->error("消息[{}]处理失败!", messsage->GetTypeName());
	}
}
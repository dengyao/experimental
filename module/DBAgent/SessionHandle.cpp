#include "SessionHandle.h"
#include <ProtobufCodec.h>
#include <proto/public_struct.pb.h>
#include <proto/server_internal.pb.h>
#include "Logging.h"
#include "AgentManager.h"
#include "ServerConfig.h"
#include "StatisticalTools.h"

SessionHandle::SessionHandle(AgentManager &manager)
	: is_logged_(false)
	, agent_manager_(manager)
{
}

// 连接事件
void SessionHandle::OnConnect()
{
}

// 接收消息事件
void SessionHandle::OnMessage(network::NetMessage &message)
{
	// 统计上行流量
	StatisticalTools::GetInstance()->AccumulateUpVolume(message.Readable() + sizeof(network::DefaultMessageFilter::MessageHeader));

	// 处理请求
	auto request = ProtubufCodec::Decode(message);
	if (request == nullptr)
	{
		agent_manager_.RespondErrorCode(*this, 0, pub::kInvalidProtocol, message);
		return;
	}

	// 连接后必须登录
	if (!is_logged_)
	{
		if (dynamic_cast<pub::PingReq*>(request.get()) == nullptr)
		{
			if (dynamic_cast<svr::LoginDBAgentReq*>(request.get()) == nullptr)
			{
				agent_manager_.RespondErrorCode(*this, 0, pub::kNotLoggedIn, message);
				logger()->warn("操作前未发起登录请求，来自{}:{}", RemoteEndpoint().address().to_string(), RemoteEndpoint().port());
			}
			else
			{
				message.Clear();
				is_logged_ = true;
				svr::LoginDBAgentRsp response;
				response.set_heartbeat_interval(ServerConfig::GetInstance()->GetHeartbeatInterval());
				ProtubufCodec::Encode(&response, message);
				Respond(message);
			}
		}
	}
	// 处理心跳包
	else if (dynamic_cast<pub::PingReq*>(request.get()) != nullptr)
	{
		message.Clear();
		pub::PongRsp response;
		ProtubufCodec::Encode(&response, message);
		Respond(message);
	}
	else
	{
		agent_manager_.HandleMessage(*this, request.get(), message);
	}
}

// 关闭事件
void SessionHandle::OnClose()
{
}

// 回复消息
void SessionHandle::Respond(const network::NetMessage &message)
{
	StatisticalTools::GetInstance()->AccumulateDownVolume(message.Readable() + sizeof(network::DefaultMessageFilter::MessageHeader));
	Send(message);
}

/************************************************************************/
// 创建消息筛选器
network::MessageFilterPointer CreateMessageFilter()
{
	return std::make_shared<network::DefaultMessageFilter>();
}

// 创建Session处理器
network::SessionHandlePointer CreateSessionHandle(AgentManager &agent_manager)
{
	return std::make_shared<SessionHandle>(agent_manager);
}
/************************************************************************/
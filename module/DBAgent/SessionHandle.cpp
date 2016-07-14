﻿#include "SessionHandle.h"
#include <proto/MessageHelper.h>
#include <proto/internal.protocol.pb.h>
#include "AgentManager.h"
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
	auto request = UnpackageMessage(message);
	if (request == nullptr)
	{
		agent_manager_.RespondErrorCode(*this, 0, internal::kInvalidProtocol, message);
		return;
	}

	// 连接后必须登录
	if (!is_logged_)
	{
		if (dynamic_cast<internal::PingReq*>(request.get()) == nullptr)
		{
			if (dynamic_cast<internal::LoginDBAgentReq*>(request.get()) == nullptr)
			{
				agent_manager_.RespondErrorCode(*this, 0, internal::kNotLoggedIn, message);
			}
			else
			{
				message.Clear();
				is_logged_ = true;
				internal::LoginDBAgentRsp response;
				response.set_heartbeat_interval(60);
				Respond(&response, message);
			}
		}
	}
	// 处理心跳包
	else if (dynamic_cast<internal::PingReq*>(request.get()) != nullptr)
	{
		message.Clear();
		internal::PongRsp response;
		Respond(&response, message);
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
void SessionHandle::Respond(google::protobuf::Message *message, network::NetMessage &buffer)
{
	PackageMessage(message, buffer);
	StatisticalTools::GetInstance()->AccumulateDownVolume(buffer.Readable() + sizeof(network::DefaultMessageFilter::MessageHeader));
	Send(buffer);
}

void SessionHandle::Respond(google::protobuf::Message *message)
{
	network::NetMessage buffer;
	Respond(message, buffer);
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
#include "SessionHandle.h"
#include <ProtobufCodec.h>
#include <proto/public_struct.pb.h>
#include <proto/server_internal.pb.h>
#include "Logging.h"
#include "GatewayManager.h"
#include "StatisticalTools.h"

SessionHandle::SessionHandle(GatewayManager &gateway_manager)
	: is_logged_(false)
	, gateway_manager_(gateway_manager)
{
}

// 连接事件
void SessionHandle::OnConnect()
{
}

// 接收消息
void SessionHandle::OnMessage(network::NetMessage &message)
{
	// 统计上行流量
	StatisticalTools::GetInstance()->AccumulateUpVolume(message.Readable() + sizeof(network::DefaultMessageFilter::MessageHeader));

	// 处理请求
	auto request = ProtubufCodec::Decode(message);
	if (request == nullptr)
	{
		return gateway_manager_.SendErrorCode(this, message, pub::kInvalidProtocol);
	}

	// 连接后必须登录
	if (!is_logged_)
	{
		if (request->GetDescriptor() != pub::PingReq::descriptor())
		{
			if (request->GetDescriptor() != svr::LoginGWReq::descriptor())
			{
				gateway_manager_.SendErrorCode(this, message, pub::kNotLoggedIn);
				logger()->warn("操作前未发起登录请求，来自{}:{}", RemoteEndpoint().address().to_string(), RemoteEndpoint().port());
				return;
			}
			else
			{
				is_logged_ = gateway_manager_.OnMessage(this, request.get(), message);
				return;
			}
		}
	}

	// 处理心跳包
	if (request->GetDescriptor() == pub::PingReq::descriptor())
	{
		message.Clear();
		pub::PongRsp response;
		ProtubufCodec::Encode(&response, message);
		Write(message);
	}
	else
	{
		gateway_manager_.OnMessage(this, request.get(), message);
	}
}

// 关闭事件
void SessionHandle::OnClose()
{
	gateway_manager_.OnClose(this);
}

// 写入消息
void SessionHandle::Write(const network::NetMessage &message)
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

network::SessionHandlePointer CreateSessionHandle(GatewayManager &gateway_manager)
{
	return std::make_shared<SessionHandle>(gateway_manager);
}
/************************************************************************/
#include "LoginSessionHandle.h"
#include <ProtobufCodec.h>
#include <proto/public_struct.pb.h>
#include "LoginConnector.h"

LoginSessionHandle::LoginSessionHandle(LoginConnector *connector, std::shared_ptr<bool> &life)
	: counter_(0)
	, connector_life_(life)
	, connector_(connector)
	, heartbeat_interval_(0)
{
}

// 获取计数器
std::shared_ptr<bool>& LoginSessionHandle::GetShared()
{
	return connector_life_;
}

// 连接成功
void LoginSessionHandle::OnConnect()
{
	if (connector_life_.unique())
	{
		Close();
	}
	else
	{
		connector_->OnConnected(this);
	}
}

// 接收消息
void LoginSessionHandle::OnMessage(network::NetMessage &message)
{
	if (connector_life_.unique())
	{
		Close();
	}
	else
	{
		connector_->OnMessage(this, message);
	}
}

// 连接关闭
void LoginSessionHandle::LoginSessionHandle::OnClose()
{
	if (!connector_life_.unique())
	{
		connector_->OnDisconnect(this);
	}
}

// 设置心跳间隔
void LoginSessionHandle::SetHeartbeatInterval(uint32_t interval)
{
	heartbeat_interval_ = interval;
	counter_ = heartbeat_interval_;
}

// 发送心跳倒计时
void LoginSessionHandle::HeartbeatCountdown()
{
	if (heartbeat_interval_ > 0)
	{
		if (--counter_ == 0)
		{
			pub::PingReq request;
			network::NetMessage message;
			ProtubufCodec::Encode(&request, message);
			Send(message);
			counter_ = heartbeat_interval_;
		}
	}
}
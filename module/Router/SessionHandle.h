#ifndef __DBPROXY_SESSION_HANDLE_H__
#define __DBPROXY_SESSION_HANDLE_H__

#include <network.h>

namespace google
{
	namespace protobuf
	{
		class Message;
	}
}

class RouterManager;

class SessionHandle : public network::TCPSessionHandler
{
public:
	SessionHandle(RouterManager &router_manager);

public:
	// 回复消息
	void Respond(network::NetMessage &message);

private:
	// 连接事件
	virtual void OnConnect() override;

	// 接收消息事件
	virtual void OnMessage(network::NetMessage &message) override;

	// 关闭事件
	virtual void OnClose() override;

private:
	bool is_logged_;
	RouterManager& router_manager_;
};

// 创建消息筛选器
network::MessageFilterPointer CreateMessageFilter();

// 创建Session处理器
network::SessionHandlePointer CreateSessionHandle(RouterManager &router_manager);

#endif

#ifndef __SESSION_HANDLE_H__
#define __SESSION_HANDLE_H__

#include <network.h>

namespace google
{
	namespace protobuf
	{
		class Message;
	}
}

class AgentManager;

class SessionHandle : public network::TCPSessionHandler
{
public:
	SessionHandle(AgentManager &manager);

public:
	// 写入消息
	void Write(const network::NetMessage &message);

private:
	// 连接事件
	virtual void OnConnect() override;

	// 接收消息事件
	virtual void OnMessage(network::NetMessage &message) override;

	// 关闭事件
	virtual void OnClose() override;

private:
	bool is_logged_;
	AgentManager& agent_manager_;
};

// 创建消息筛选器
network::MessageFilterPointer CreateMessageFilter();

// 创建Session处理器
network::SessionHandlePointer CreateSessionHandle(AgentManager &agent_manager);

#endif

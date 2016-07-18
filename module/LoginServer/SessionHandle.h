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

enum class RoleType
{
	kUser = 1,
	kLinker = 2,
};

class LoginManager;

class SessionHandle : public network::TCPSessionHandler
{
public:
	SessionHandle(LoginManager &login_manager);

private:
	// 连接事件
	virtual void OnConnect() override;

	// 接收消息事件
	virtual void OnMessage(network::NetMessage &message) override;

	// 关闭事件
	virtual void OnClose() override;

private:
	RoleType      type_;
	LoginManager& login_manager_;
};

// 创建消息筛选器
network::MessageFilterPointer CreateMessageFilter();

// 创建Session处理器
network::SessionHandlePointer CreateSessionHandle(LoginManager &login_manager);

#endif

#ifndef __SESSION_HANDLE_H__
#define __SESSION_HANDLE_H__

#include <network.h>

class LinkerManager;

class SessionHandle : public network::TCPSessionHandler
{
public:
	SessionHandle(LinkerManager &linker_manager);

public:
	bool IsAuthTimeout() const;

public:
	virtual void OnConnect() override;

	virtual void OnMessage(network::NetMessage &message) override;

	virtual void OnClose() override;
	
private:
	LinkerManager&							linker_manager_;
	std::chrono::steady_clock::time_point	connect_time_;
};

// 创建消息筛选器
network::MessageFilterPointer CreateMessageFilter();

// 创建Session处理器
network::SessionHandlePointer CreateSessionHandle(LinkerManager &linker_manager);

#endif

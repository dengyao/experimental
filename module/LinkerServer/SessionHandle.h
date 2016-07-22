#ifndef __SESSION_HANDLE_H__
#define __SESSION_HANDLE_H__

#include <network.h>

class LinkerManager;

class SessionHandle : public network::TCPSessionHandler
{
public:
	SessionHandle(LinkerManager &linker_manager);

public:
	virtual void OnConnect() = 0;

	virtual void OnMessage(network::NetMessage &message) = 0;

	virtual void OnClose() = 0;
	
private:
	bool									is_logged_;
	LinkerManager&							linker_manager_;
	std::chrono::steady_clock::time_point	connect_time_;
};

// 创建消息筛选器
network::MessageFilterPointer CreateMessageFilter();

// 创建Session处理器
network::SessionHandlePointer CreateSessionHandle(LinkerManager &linker_manager);

#endif

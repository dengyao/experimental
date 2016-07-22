#ifndef __LINKER_MANAGER_H__
#define __LINKER_MANAGER_H__

#include <network.h>

class LinkerManager
{
public:
	LinkerManager(network::IOServiceThreadManager &threads);

public:
	// 处理来自网关的消息
	void HandleMessageFromRouter();

	// 处理来自登录服务器的消息
	void HandleMessageFromLoginServer();

private:
	// 更新定时器
	void OnUpdateTimer(asio::error_code error_code);

private:
	asio::steady_timer                          timer_;
	const std::function<void(asio::error_code)> wait_handler_;
	network::IOServiceThreadManager&            threads_;
};

#endif
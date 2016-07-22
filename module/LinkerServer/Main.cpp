#include <iostream>
#include <network.h>
#include <common/Path.h>
#include "Logging.h"
#include "ServerConfig.h"
#include "LoginConnector.h"

std::unique_ptr<network::TCPServer> g_server;
network::IOServiceThreadManager*    g_thread_manager = nullptr;

network::MessageFilterPointer CreateMessageFilter()
{
	return std::make_shared<network::DefaultMessageFilter>();
}

int main(int argc, char *argv[])
{
	// 初始化日志设置
	if (!OnceInitLogSettings("logs", path::split(*argv)[1]))
	{
		std::cerr << "初始化日志设置失败!" << std::endl;
		assert(false);
		exit(-1);
	}
	logger()->info("初始化日志设置成功!");

	// 加载服务器配置
	std::string fullpath = path::curdir() + path::sep + "config.LinkerServer.json";
	if (!ServerConfig::GetInstance()->Load(fullpath))
	{
		logger()->critical("加载服务器配置失败！");
		assert(false);
		exit(-1);
	}
	logger()->info("加载服务器配置成功!");
	ServerConfig::GetInstance()->ProcessName(path::basename(*argv));

	// 连接登录服务器
	network::IOServiceThreadManager threads(ServerConfig::GetInstance()->GetUseThreadNum());
	g_thread_manager = &threads;

	std::unique_ptr<LoginConnector> db_client;
	asio::ip::tcp::endpoint endpoint(asio::ip::address_v4::from_string(ServerConfig::GetInstance()->GetLoginSeverIP()),
		ServerConfig::GetInstance()->GetLoginServerPort());

	std::function<void(uint16_t)> callback = [](uint16_t)
	{

	};
	db_client = std::make_unique<LoginConnector>(threads, endpoint, callback);

	threads.Run();

	// 连接路由服务器

	// 启动Linker服务器

	system("pause");
	return 0;
}
#include <iostream>
#include <network.h>
#include <Connector.h>
#include <common/Path.h>
#include <proto/server_internal.pb.h>
#include "Logging.h"
#include "ServerConfig.h"
#include "GlobalObject.h"
#include "LoginConnector.h"

std::unique_ptr<network::TCPServer> g_server;
network::IOServiceThreadManager*    g_thread_manager = nullptr;


// 运行服务器
void RunLoginServer()
{
	
}

// 连接路由服务器
void ConnectRouter(uint16_t linker_id)
{
	if (linker_id == 0)
	{
		logger()->critical("连接登录服务器失败!");
		assert(false);
		exit(-1);
	}
	else
	{
		logger()->info("连接登录服务器成功!");

		std::unique_ptr<router::Connector> connector;
		try
		{
			asio::ip::tcp::endpoint endpoint(asio::ip::address_v4::from_string(ServerConfig::GetInstance()->GetRouterIP()),
				ServerConfig::GetInstance()->GetRouterPort());
			connector = std::make_unique<router::Connector>(*g_thread_manager, endpoint, ServerConfig::GetInstance()->GetRouterConnections(), svr::kLinkerServer, linker_id);
			logger()->info("连接路由服务器成功!");
		}
		catch (...)
		{
			logger()->critical("连接路由服务器失败!");
			assert(false);
			exit(-1);
		}
		OnceInitGlobalConnector(std::move(connector));
		RunLoginServer();
	}
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
		logger()->critical("加载服务器配置失败!");
		assert(false);
		exit(-1);
	}
	logger()->info("加载服务器配置成功!");
	ServerConfig::GetInstance()->ProcessName(path::basename(*argv));

	// 连接登录服务器
	network::IOServiceThreadManager threads(ServerConfig::GetInstance()->GetUseThreadNum());
	g_thread_manager = &threads;

	std::unique_ptr<LoginConnector> login_connector;
	asio::ip::tcp::endpoint endpoint(asio::ip::address_v4::from_string(ServerConfig::GetInstance()->GetLoginSeverIP()),
		ServerConfig::GetInstance()->GetLoginServerPort());
	try
	{
		login_connector = std::make_unique<LoginConnector>(threads, endpoint, ConnectRouter);
		OnceInitGlobalLoginConnector(std::move(login_connector));
	}
	catch (...)
	{
		logger()->critical("连接登录服务器失败!");
		assert(false);
		exit(-1);
	}
	threads.Run();

	return 0;
}
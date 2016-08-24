#include <iostream>
#include <network.h>
#include <GWClient.h>
#include <common/Path.h>
#include <proto/server_internal.pb.h>
#include "Logging.h"
#include "ServerConfig.h"
#include "GlobalObject.h"
#include "LinkerManager.h"
#include "SessionHandle.h"
#include "LoginConnector.h"

std::unique_ptr<network::TCPServer> g_server;
std::unique_ptr<LinkerManager>      g_linker_manager;
network::IOServiceThreadManager*    g_thread_manager = nullptr;

// 运行Linker服务器
void RunLoginServer()
{
	g_linker_manager = std::make_unique<LinkerManager>(*g_thread_manager);
	asio::ip::tcp::endpoint endpoint(asio::ip::address_v4(), ServerConfig::GetInstance()->GetPort());

	g_server = std::make_unique<network::TCPServer>(endpoint, *g_thread_manager,
		std::bind(CreateSessionHandle, std::ref(*g_linker_manager.get())),
		CreateMessageFilter, ServerConfig::GetInstance()->GetHeartbeatInterval());

	GlobalLoginConnector()->SetMessageCallback(std::bind(&LinkerManager::OnLoginServerMessage, g_linker_manager.get(),
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	GlobalGWClient()->SetMessageCallback(std::bind(&LinkerManager::OnGatewayServerMessage, g_linker_manager.get(),
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	logger()->info("Linker服务器[ip:{} port:{}]启动成功!", g_server->LocalEndpoint().address().to_string().c_str(), g_server->LocalEndpoint().port());
}

// 连接网关服务器
void ConnectGatewayServer(uint16_t linker_id)
{
	if (linker_id == 0)
	{
		logger()->critical("与登录服务器验证失败!");
		assert(false);
		exit(-1);
	}
	else
	{
		logger()->info("与登录服务器验证成功!");

		std::unique_ptr<gw::GWClient> connector;
		try
		{
			asio::ip::tcp::endpoint endpoint(asio::ip::address_v4::from_string(ServerConfig::GetInstance()->GetGWServerIP()),
				ServerConfig::GetInstance()->GetGWServerPort());
			connector = std::make_unique<gw::GWClient>(*g_thread_manager, endpoint, ServerConfig::GetInstance()->GetGWServerConnections(), svr::kLinkerServer, linker_id);
			logger()->info("连接网关服务器成功!");
		}
		catch (...)
		{
			logger()->critical("连接网关服务器失败!");
			assert(false);
			exit(-1);
		}
		OnceInitGlobalGatewayConnector(std::move(connector));
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
		login_connector = std::make_unique<LoginConnector>(threads, endpoint, ConnectGatewayServer);
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
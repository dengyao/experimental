#include <iostream>
#include <network.h>
#include <DBClient.h>
#include <GWClient.h>
#include <common/Path.h>
#include <proto/server_internal.pb.h>
#include "Logging.h"
#include "ServerConfig.h"
#include "GlobalObject.h"
#include "LogicManager.h"

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
	std::string fullpath = path::curdir() + path::sep + "config.LogicServer.json";
	if (!ServerConfig::GetInstance()->Load(fullpath))
	{
		logger()->critical("加载服务器配置失败!");
		assert(false);
		exit(-1);
	}
	logger()->info("加载服务器配置成功!");
	ServerConfig::GetInstance()->ProcessName(path::basename(*argv));

	network::IOServiceThreadManager threads(ServerConfig::GetInstance()->GetUseThreadNum());

	// 连接数据库代理
	try
	{
		std::unique_ptr<db::DBClient> db_client;
		asio::ip::tcp::endpoint endpoint(asio::ip::address_v4::from_string(ServerConfig::GetInstance()->GetDBAgentIP()),
			ServerConfig::GetInstance()->GetDBAgentPort());
		db_client = std::make_unique<db::DBClient>(threads, endpoint, ServerConfig::GetInstance()->GetDBAgentConnections());
		OnceInitGlobalDBClient(std::move(db_client));
		logger()->info("连接数据库代理服务器成功！");
	}
	catch (...)
	{
		logger()->critical("连接数据库代理服务器失败！");
		assert(false);
		exit(-1);
	}

	// 连接网关服务器
	try
	{
		std::unique_ptr<gateway::GatewayClient> connector;
		asio::ip::tcp::endpoint endpoint(asio::ip::address_v4::from_string(ServerConfig::GetInstance()->GetGWServerIP()),
			ServerConfig::GetInstance()->GetGWServerPort());
		connector = std::make_unique<gateway::GatewayClient>(threads, endpoint, ServerConfig::GetInstance()->GetGWServerConnections(), svr::kMainLogicSever);
		OnceInitGlobalGatewayConnector(std::move(connector));
		logger()->info("连接网关服务器成功!");
	}
	catch (...)
	{
		logger()->critical("连接网关服务器失败!");
		assert(false);
		exit(-1);
	}
	threads.Run();

	return 0;
}
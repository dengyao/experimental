#include <vector>
#include <memory>
#include <iostream>
#include <common/Path.h>
#include <common/TaskPools.h>
#include <common/StringHelper.h>
#include "Logging.h"
#include "AgentMySQL.h"
#include "AgentManager.h"
#include "ServerConfig.h"
#include "SessionHandle.h"

// 创建数据库连接
std::vector<ConnectorPointer> CreateMySQLConnector()
{
	std::vector<ConnectorPointer> connectors;
	auto container = ServerConfig::GetInstance()->GetMySQLConnectionInfo();
	for (size_t i = 0; i < container.size(); ++i)
	{
		for (size_t j = 0; j < container[i].connections; ++j)
		{
			ConnectorPointer connector;
			try
			{
				ErrorCode error_code;
				connector = std::make_unique<ConnectorMySQL>(ServerConfig::GetInstance()->GetMySQLHost(),
					ServerConfig::GetInstance()->GetMySQLPort(),
					ServerConfig::GetInstance()->GetMySQLUser(),
					ServerConfig::GetInstance()->GetMySQLPassword(),
					5);

				static_cast<ConnectorMySQL*>(connector.get())->SelectDatabase(container[i].db_name.c_str(), error_code);
				if (error_code)
				{
					logger()->critical(error_code.Message());
					assert(false);
					exit(-1);
				}

				static_cast<ConnectorMySQL*>(connector.get())->SetCharacterSet(ServerConfig::GetInstance()->GetMySQLCharset(), error_code);
				if (error_code)
				{
					logger()->critical(error_code.Message());
					assert(false);
					exit(-1);
				}
				connectors.push_back(std::move(connector));
			}
			catch (...)
			{
				logger()->critical("连接MySQL数据库失败!");
				assert(false);
				exit(-1);
			}
		}
	}

	if (connectors.empty())
	{
		logger()->critical("未创建任何数据库连接!");
		assert(false);
		exit(-1);
	}

	return connectors;
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
	std::string fullpath = path::curdir() + path::sep + "config.DBAgent.json";
	if (!ServerConfig::GetInstance()->Load(fullpath))
	{
		logger()->critical("加载服务器配置失败!");
		assert(false);
		exit(-1);
	}
	logger()->info("加载服务器配置成功!");
	ServerConfig::GetInstance()->ProcessName(path::basename(*argv));

	// 启动数据库代理
	TaskPools pools(ServerConfig::GetInstance()->GetDBUseThreadNum());
	network::IOServiceThreadManager threads(ServerConfig::GetInstance()->GetUseThreadNum());

	AgentMySQL mysql_agent(CreateMySQLConnector(), pools, ServerConfig::GetInstance()->GetMaxConnectionRequestBacklog());
	AgentManager manager(threads, mysql_agent, ServerConfig::GetInstance()->GetMaxRequestBacklog());

	asio::ip::tcp::endpoint endpoint(asio::ip::address_v4(), ServerConfig::GetInstance()->GetPort());
	network::TCPServer server(endpoint, threads, std::bind(CreateSessionHandle, std::ref(manager)),
		CreateMessageFilter, ServerConfig::GetInstance()->GetHeartbeatInterval());
	logger()->info("数据库代理[ip:{} port:{}]启动成功!", server.LocalEndpoint().address().to_string().c_str(), server.LocalEndpoint().port());
	threads.Run();
	
	return 0;
}
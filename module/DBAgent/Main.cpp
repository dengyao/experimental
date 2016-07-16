#include <vector>
#include <memory>
#include <iostream>
#include <common/Path.h>
#include <common/TaskPools.h>
#include <common/StringHelper.h>
#include "AgentImpl.h"
#include "AgentManager.h"
#include "ServerConfig.h"
#include "SessionHandle.h"

// 创建数据库连接
std::vector<std::unique_ptr<ConnectorMySQL>> CreateMySQLConnector()
{
	std::vector<std::unique_ptr<ConnectorMySQL>> connectors;
	auto container = ServerConfig::GetInstance()->GetMySQLConnectionInfo();
	for (size_t i = 0; i < container.size(); ++i)
	{
		for (size_t j = 0; j < container[i].connections; ++j)
		{
			std::unique_ptr<ConnectorMySQL> connector;
			try
			{
				ErrorCode error_code;
				connector = std::make_unique<ConnectorMySQL>(ServerConfig::GetInstance()->GetMySQLHost(),
					ServerConfig::GetInstance()->GetMySQLPort(),
					ServerConfig::GetInstance()->GetMySQLUser(),
					ServerConfig::GetInstance()->GetMySQLPassword(),
					5);

				connector->SelectDatabase("sgs", error_code);
				if (error_code)
				{
					connectors.clear();
					std::cerr << error_code.Message() << std::endl;
					exit(-1);
				}

				connector->SetCharacterSet("utf8", error_code);
				if (error_code)
				{
					connectors.clear();
					std::cerr << error_code.Message() << std::endl;
					exit(-1);
				}
				connectors.push_back(std::move(connector));
			}
			catch (const std::exception&)
			{
				connectors.clear();
				std::cerr << "连接数据库失败!" << std::endl;
				exit(-1);
			}
		}
	}
	return connectors;
}

int main(int argc, char *argv[])
{
	// 加载服务器配置
	std::string fullpath = path::curdir() + path::sep + "config.DBAgent.json";
	if (!ServerConfig::GetInstance()->Load(fullpath))
	{
		std::cout << "加载服务器配置失败！" << std::endl;
		assert(false);
		exit(-1);
	}
	ServerConfig::GetInstance()->ProcessName(path::basename(*argv));

	// 启动数据库代理
	TaskPools pools(ServerConfig::GetInstance()->GetDBUseThreadNum());
	network::IOServiceThreadManager threads(ServerConfig::GetInstance()->GetUseThreadNum());

	AgentImpl<MySQL> mysql_agent(CreateMySQLConnector(), pools, ServerConfig::GetInstance()->GetMaxConnectionRequestBacklog());
	AgentManager manager(threads, mysql_agent, ServerConfig::GetInstance()->GetMaxRequestBacklog());

	asio::ip::tcp::endpoint endpoint(asio::ip::address_v4(), ServerConfig::GetInstance()->GetPort());
	network::TCPServer server(endpoint, threads, std::bind(CreateSessionHandle, std::ref(manager)), CreateMessageFilter);
	std::cout << string_helper::format("数据库代理[ip:%s port:%u]启动成功!", server.LocalEndpoint().address().to_string().c_str(), server.LocalEndpoint().port()) << std::endl;
	threads.Run();
	
	return 0;
}
#include <vector>
#include <memory>
#include <iostream>
#include <common.h>
#include "AgentImpl.h"
#include "AgentManager.h"
#include "SessionHandle.h"

// 创建MySQL连接器
std::vector<std::unique_ptr<ConnectorMySQL>> CreateConnectorMySQL(const size_t num)
{
	assert(num > 0);
	std::vector<std::unique_ptr<ConnectorMySQL>> connector_lists;
	for (size_t i = 0; i < num; ++i)
	{
		std::unique_ptr<ConnectorMySQL> connector;
		try
		{
			ErrorCode error_code;
			connector = std::make_unique<ConnectorMySQL>("192.168.1.201", 3306, "root", "123456", 5);

			connector->SelectDatabase("sgs", error_code);
			if (error_code)
			{
				std::cerr << error_code.Message() << std::endl;
			}

			connector->SetCharacterSet("utf8", error_code);
			if (error_code)
			{
				std::cerr << error_code.Message() << std::endl;
			}

			connector_lists.push_back(std::move(connector));
		}
		catch (const std::exception&)
		{
			std::cerr << "连接数据库失败!" << std::endl;
			getchar();
			exit(0);
		}
	}
	return connector_lists;
}

int main(int argc, char *argv[])
{
	TaskPools pools(std::thread::hardware_concurrency() / 2);
	network::IOServiceThreadManager threads(std::thread::hardware_concurrency() / 2);

	AgentImpl<MySQL> mysql_proxy(CreateConnectorMySQL(8), pools, 1000000);	// 最后个参数为单个连接最大积压数量
	AgentManager manager(threads, mysql_proxy, 102400);    // 最后个参数为服务器最大积压数量

	asio::ip::tcp::endpoint endpoint(asio::ip::address_v4(), 4235);
	network::TCPServer server(endpoint, threads, std::bind(CreateSessionHandle, std::ref(manager)), CreateMessageFilter);
	threads.Run();
	
	return 0;
}
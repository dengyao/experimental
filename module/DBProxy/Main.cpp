#include <iostream>
#include <vector>
#include <memory>
#include <base.h>
#include "ProxyImpl.h"
#include "ConnectorMySQL.h"
#include "SessionHandle.h"
#include "ProxyManager.h"

// 创建MySQL连接器
std::vector<std::unique_ptr<dbproxy::ConnectorMySQL>> CreateConnectorMySQL(const size_t num)
{
	assert(num > 0);
	std::vector<std::unique_ptr<dbproxy::ConnectorMySQL>> connector_lists;
	for (size_t i = 0; i < num; ++i)
	{
		std::unique_ptr<dbproxy::ConnectorMySQL> connector;
		try
		{
			dbproxy::ErrorCode error_code;
			connector = std::make_unique<dbproxy::ConnectorMySQL>("192.168.1.201", 3306, "root", "123456", 5);

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
	eddy::IOServiceThreadManager threads(std::thread::hardware_concurrency() / 2);

	dbproxy::ProxyImpl<dbproxy::MySQL> mysql_proxy(CreateConnectorMySQL(8), pools, 1000000);
	dbproxy::ProxyManager handler(threads, mysql_proxy);

	asio::ip::tcp::endpoint endpoint(asio::ip::address_v4(), 4235);
	eddy::TCPServer server(endpoint, threads, std::bind(dbproxy::CreateSessionHandle, std::ref(handler)), dbproxy::CreateMessageFilter);
	threads.Run();
	
	return 0;
}
#include <iostream>
#include "ProcessManager.h"

class SessionHandle : public eddy::TCPSessionHandler
{
public:
	SessionHandle(ProcessManager &process)
		: process_(process)
	{
	}

	virtual void OnConnect() override
	{
	}

	virtual void OnMessage(eddy::NetMessage &message) override
	{
		process_.HandleMessage(*this, message);
	}

	virtual void OnClose() override
	{
	}

private:
	ProcessManager& process_;
};

class MyCreator
{
public:
	MyCreator(ProcessManager &process)
		: process_(process)
	{
	}

	std::shared_ptr<SessionHandle> CreateSessionHandle()
	{
		return std::make_shared<SessionHandle>(process_);
	}

	std::shared_ptr<eddy::DefaultMessageFilter> CreateMessageFilter()
	{
		return std::make_shared<eddy::DefaultMessageFilter>();
	}

private:
	ProcessManager& process_;
};

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

	dbproxy::ProxyManager<dbproxy::MySQL> mysql_proxy(CreateConnectorMySQL(8), pools, 1000000);
	ProcessManager handler(threads, mysql_proxy);

	MyCreator creator(handler);
	asio::ip::tcp::endpoint endpoint(asio::ip::address_v4(), 4235);
	eddy::TCPServer server(endpoint, threads, std::bind(&MyCreator::CreateSessionHandle, &creator), std::bind(&MyCreator::CreateMessageFilter, &creator));
	threads.Run();
	
	return 0;
}
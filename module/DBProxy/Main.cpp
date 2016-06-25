#include "eddy.h"
#include "Types.h"
#include "ProxyManager.h"
#include "ConnectorMySQL.h"
#include "proto/db.request.pb.h"
#include "proto/db.response.pb.h"

class CentralProcessor
{
	struct RequestSource
	{
		int number;
		eddy::TCPSessionID id;
		RequestSource(int args1, eddy::TCPSessionID args2)
			: number(args1)
			, id(args2)
		{

		}
	};

public:
	CentralProcessor(eddy::IOServiceThreadManager &threads, dbproxy::ProxyManager<dbproxy::MySQL> &mysql)
		: threads_(threads)
		, generator_(65535)
		, mysql_proxy_(mysql)
	{
		threads_.MainThread()->IOService();
	}

	void OnMessage(eddy::TCPSessionID id, eddy::NetMessage &message)
	{
		proto_db::Request req;
		if (req.ParseFromArray(message.Data(), message.Readable()))
		{
			uint32_t number = 0;
			if (generator_.Get(number))
			{
				requests_.insert(std::make_pair(number, RequestSource(req.number(), id)));
				mysql_proxy_.Append(number, (dbproxy::ActionType)req.action(), req.dbname().c_str(), req.statement().c_str(), req.statement().size());
			}
		}
		else
		{
			assert(false);
		}
	}

	void OnRefresh()
	{
		assert(completion_list_.empty());
		if (mysql_proxy_.GetCompletionQueue(completion_list_) > 0)
		{
			
		}
	}

public:
	eddy::IOServiceThreadManager&          threads_;
	std::map<uint32_t, RequestSource>      requests_;
	eddy::IDGenerator                      generator_;
	dbproxy::ProxyManager<dbproxy::MySQL>& mysql_proxy_;
	std::vector<dbproxy::Result>           completion_list_;
};

class SessionHandle : public eddy::TCPSessionHandle
{
public:
	virtual void OnConnect() override
	{

	}

	virtual void OnMessage(eddy::NetMessage &message) override
	{
		processor_.OnMessage(SessionID(), message);
	}

	virtual void OnClose() override
	{

	}
};

std::shared_ptr<SessionHandle> CreateSessionHandle()
{
	return std::make_shared<SessionHandle>();
}

std::shared_ptr<eddy::DefaultMessageFilter> CreateMessageFilter()
{
	return std::make_shared<eddy::DefaultMessageFilter>();
}

int main(int argc, char **argv)
{
	asio::ip::tcp::endpoint endpoint(asio::ip::address_v4(), 4235);
	eddy::IOServiceThreadManager threads(/*std::thread::hardware_concurrency()*/1);
	eddy::TCPServer server(endpoint, threads, CreateSessionHandle, CreateMessageFilter);
	threads.Run();
	
	return 0;
}

//#include <iostream>
//
//class Duration
//{
//public:
//	Duration()
//		: start_time_(std::chrono::system_clock::now())
//	{}
//
//	void reset()
//	{
//		start_time_ = std::chrono::system_clock::now();
//	}
//
//	std::chrono::seconds::rep seconds()
//	{
//		return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start_time_).count();
//	}
//
//	std::chrono::milliseconds::rep milli_seconds()
//	{
//		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time_).count();
//	}
//
//	std::chrono::microseconds::rep micro_seconds()
//	{
//		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time_).count();
//	}
//
//	std::chrono::nanoseconds::rep nano_seconds()
//	{
//		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - start_time_).count();
//	}
//
//private:
//	std::chrono::system_clock::time_point start_time_;
//};
//
//const int sum = 3000000;
//const char *sql = "SELECT id FROM `actor` WHERE id=3983617;";
//
//void query(dbproxy::ProxyManager<dbproxy::MySQL> &proxy)
//{
//	for (int i = 1; i <= sum; ++i)
//	{
//		if (!proxy.Append(i, dbproxy::ActionType::kSelect, "sgs", sql, strlen(sql)))
//		{
//			assert(false);
//		}
//	}
//}
//
//
//int main(int argc, char *argv[])
//{
//	std::vector<std::unique_ptr<dbproxy::ConnectorMySQL>> vec;
//	for (int i = 0; i < 8; ++i)
//	{
//		std::unique_ptr<dbproxy::ConnectorMySQL> connector;
//		try
//		{
//			dbproxy::ErrorCode error_code;
//			connector = std::make_unique<dbproxy::ConnectorMySQL>("192.168.1.201", 3306, "root", "123456", 5);
//
//			connector->SelectDatabase("sgs", error_code);
//			if (error_code)
//			{
//				std::cout << error_code.Message() << std::endl;
//			}
//
//			connector->SetCharacterSet("utf8", error_code);
//			if (error_code)
//			{
//				std::cout << error_code.Message() << std::endl;
//			}
//
//			vec.push_back(std::move(connector));
//		}
//		catch (const std::exception &e)
//		{
//			std::cout << e.what() << std::endl;
//			system("pause");
//		}
//	}
//
//	std::shared_ptr<TaskPools> pools = std::make_shared<TaskPools>(4);
//	dbproxy::ProxyManager<dbproxy::MySQL> proxy(std::move(vec), pools, 10000000);
//	std::thread td(std::bind(query, std::ref(proxy)));
//
//	Duration duration;
//	std::vector<dbproxy::Result> completion;
//	int count = 0;
//	while (true)
//	{
//		Sleep(1000);
//		if (proxy.GetCompletionQueue(completion) > 0)
//		{
//			std::cout << completion.size() << std::endl;
//			count += completion.size();
//			completion.clear();
//		}
//
//		if (count == sum)
//		{
//			break;
//		}
//	}
//	std::cout << "本次耗时" << duration.seconds() << "秒" << std::endl;
//
//	std::cout << "操作完成" << std::endl;
//	system("pause");
//	return 0;
//}
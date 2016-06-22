#include <iostream>
#include <rapidjson/document.h>
#include <DBProxy/Private/ConnectorMySQL.h>
#include <DBProxy/Private/ProxyManager.h>
#include <functional>
#include <DBProxy/Private/TaskPools.h>

class Duration
{
public:
	Duration()
		: start_time_(std::chrono::system_clock::now())
	{}

	void reset()
	{
		start_time_ = std::chrono::system_clock::now();
	}

	std::chrono::seconds::rep seconds()
	{
		return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start_time_).count();
	}

	std::chrono::milliseconds::rep milli_seconds()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time_).count();
	}

	std::chrono::microseconds::rep micro_seconds()
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time_).count();
	}

	std::chrono::nanoseconds::rep nano_seconds()
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - start_time_).count();
	}

private:
	std::chrono::system_clock::time_point start_time_;
};

const int sum = 300000;
const char *sql = "SELECT id FROM `actor` WHERE id=3983617;";

void query(dbproxy::ProxyManager<dbproxy::MySQL> &proxy)
{	
	for (int i = 1; i <= sum; ++i)
	{
		if (!proxy.Append(i, dbproxy::CommandType::kSelect, "sgs", sql, strlen(sql)))
		{
			assert(false);
		}
	}
}

int main(int argc, char *argv[])
{
	std::vector<std::unique_ptr<dbproxy::ConnectorMySQL>> vec;
	for (int i = 0; i < 16; ++i)
	{
		std::unique_ptr<dbproxy::ConnectorMySQL> connector;
		try
		{
			dbproxy::ErrorCode error_code;
			connector = std::make_unique<dbproxy::ConnectorMySQL>("192.168.1.201", 3306, "root", "123456", 5);

			connector->SelectDatabase("sgs", error_code);
			if (error_code)
			{
				std::cout << error_code.Message() << std::endl;
			}

			connector->SetCharacterSet("utf8", error_code);
			if (error_code)
			{
				std::cout << error_code.Message() << std::endl;
			}

			vec.push_back(std::move(connector));
		}
		catch (const std::exception &e)
		{
			std::cout << e.what() << std::endl;
			system("pause");
		}
		
	}

	std::shared_ptr<TaskPools> pools = std::make_shared<TaskPools>(15);
	dbproxy::ProxyManager<dbproxy::MySQL> proxy(std::move(vec), pools, 10000000);
	std::thread td(std::bind(query, std::ref(proxy)));
	
	Duration duration;
	std::vector<decltype(proxy)::Result> completion;
	int count = 0;
	while (true)
	{
		if (proxy.GetCompletionQueue(completion) > 0)
		{
			count += completion.size();
			completion.clear();
		}

		if (count == sum)
		{
			break;
		}
	}
	std::cout << "本次耗时" << duration.seconds() << "秒" << std::endl;

	std::cout << "操作完成" << std::endl;
	system("pause");
	return 0;
}
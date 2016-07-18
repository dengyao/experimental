#include <iostream>
#include <DBClient.h>
#include <DBResult.h>
#include <common/Path.h>
#include <proto/internal.pb.h>
#include "Logging.h"

struct SPartition
{
	int id;
	std::string name;
	int status;
	bool is_recommend;
	std::string createtime;

	SPartition()
		: id(0), status(0), is_recommend(false)
	{
	}
};

// 查询分区信息
void QueryPartitionInfo(DBClient *client)
{
	client->AsyncSelect(kMySQL, "db_verify", "SELECT * FROM `partition`;", [](google::protobuf::Message *message)
	{
		if (dynamic_cast<internal::QueryDBAgentRsp*>(message) != nullptr)
		{
			std::vector<SPartition> partition_lists;
			auto rsp = static_cast<internal::QueryDBAgentRsp*>(message);
			DBResult result(rsp->result().data(), rsp->result().size());
			for (unsigned int row = 0; row < result.NumRows(); ++row)
			{
				SPartition partition;
				partition.id = atoi(result.Value(row, 0));
				partition.name = result.Value(row, 1);
				partition.status = atoi(result.Value(row, 2));
				partition.is_recommend = atoi(result.Value(row, 3)) != 0;
				partition.createtime = result.Value(row, 4);
				partition_lists.push_back(partition);
			}
			logger()->info("查询分区信息成功，共有{}个分区", partition_lists.size());
		}
		else
		{
			if (dynamic_cast<internal::DBErrorRsp*>(message) != nullptr)
			{
				auto error = static_cast<internal::DBErrorRsp*>(message);
				logger()->critical("查询分区信息失败，error code: {} {}", error->error_code(), error->what());
			}
			else if(dynamic_cast<internal::DBAgentErrorRsp*>(message) != nullptr)
			{
				auto error = static_cast<internal::DBAgentErrorRsp*>(message);
				logger()->critical("查询分区信息失败，error code: {}", error->error_code());
			}
			assert(false);
			exit(-1);
		}
	});
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

	// 查询分区信息
	network::IOServiceThreadManager threads(5);
	asio::ip::tcp::endpoint endpoint(asio::ip::address_v4::from_string("192.168.1.109"), 10000);
	DBClient db_client(threads, endpoint);
	QueryPartitionInfo(&db_client);
	threads.Run();

	return 0;
}
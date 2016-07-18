#include <iostream>
#include <DBClient.h>
#include <DBResult.h>
#include <common/Path.h>
#include <proto/server_internal.pb.h>
#include "Logging.h"
#include "ServerConfig.h"
#include "LoginManager.h"
#include "SessionHandle.h"

std::unique_ptr<network::TCPServer> g_server;
std::unique_ptr<LoginManager>       g_login_manager;
network::IOServiceThreadManager*    g_thread_manager = nullptr;

// 运行服务器
void RunLoginServer(const std::vector<SPartition> &partition)
{
	assert(g_thread_manager != nullptr);
	if (g_thread_manager != nullptr)
	{
		asio::ip::tcp::endpoint endpoint(asio::ip::address_v4(), ServerConfig::GetInstance()->GetPort());
		g_login_manager = std::make_unique<LoginManager>(*g_thread_manager, partition);
		g_server = std::make_unique<network::TCPServer>(endpoint, *g_thread_manager, std::bind(CreateSessionHandle,
			std::ref(*g_login_manager.get())), CreateMessageFilter);
		logger()->info("登录服务器[ip:{} port:{}]启动成功!", g_server->LocalEndpoint().address().to_string().c_str(), g_server->LocalEndpoint().port());
	}
	else
	{
		logger()->info("运行登录服务器失败!");
		exit(-1);
	}
}

// 查询分区信息
void QueryPartitionInfo(DBClient *client)
{
	client->AsyncSelect(kMySQL, "db_verify", "SELECT * FROM `partition`;", [](google::protobuf::Message *message)
	{
		if (dynamic_cast<svr::QueryDBAgentRsp*>(message) != nullptr)
		{
			std::vector<SPartition> partition_lists;
			auto rsp = static_cast<svr::QueryDBAgentRsp*>(message);
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
			RunLoginServer(partition_lists);
		}
		else
		{
			if (dynamic_cast<svr::DBErrorRsp*>(message) != nullptr)
			{
				auto error = static_cast<svr::DBErrorRsp*>(message);
				logger()->critical("查询分区信息失败，error code: {} {}", error->error_code(), error->what());
			}
			else if(dynamic_cast<svr::DBAgentErrorRsp*>(message) != nullptr)
			{
				auto error = static_cast<svr::DBAgentErrorRsp*>(message);
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

	// 加载服务器配置
	std::string fullpath = path::curdir() + path::sep + "config.LoginServer.json";
	if (!ServerConfig::GetInstance()->Load(fullpath))
	{
		logger()->critical("加载服务器配置失败！");
		assert(false);
		exit(-1);
	}
	logger()->info("加载服务器配置成功!");
	ServerConfig::GetInstance()->ProcessName(path::basename(*argv));

	// 查询分区信息
	network::IOServiceThreadManager threads(ServerConfig::GetInstance()->GetUseThreadNum());
	g_thread_manager = &threads;
	asio::ip::tcp::endpoint endpoint(asio::ip::address_v4::from_string(ServerConfig::GetInstance()->GetDBAgentIP()),
		ServerConfig::GetInstance()->GetDBAgentPort());
	DBClient db_client(threads, endpoint, ServerConfig::GetInstance()->GetDBAgentConnections());
	QueryPartitionInfo(&db_client);
	threads.Run();

	return 0;
}
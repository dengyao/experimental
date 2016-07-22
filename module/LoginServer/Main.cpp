#include <iostream>
#include <DBClient.h>
#include <DBResult.h>
#include <common/Path.h>
#include <proto/server_internal.pb.h>
#include "Logging.h"
#include "GlobalObject.h"
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
			std::ref(*g_login_manager.get())), CreateMessageFilter, ServerConfig::GetInstance()->GetHeartbeatInterval());
		logger()->info("登录服务器[ip:{} port:{}]启动成功!", g_server->LocalEndpoint().address().to_string().c_str(), g_server->LocalEndpoint().port());
	}
	else
	{
		logger()->info("运行登录服务器失败!");
		exit(-1);
	}
}

// 查询分区信息
void QueryPartitionInfo()
{
	auto callback = [](google::protobuf::Message *message)
	{
		if (dynamic_cast<svr::QueryDBAgentRsp*>(message) != nullptr)
		{
			std::vector<SPartition> partition_lists;
			auto response = static_cast<svr::QueryDBAgentRsp*>(message);
			db::WrapResultSet result_set(response->result().data(), response->result().size());
			for (unsigned int row = 0; row < result_set.NumRows(); ++row)
			{
				SPartition partition;
				db::WrapResultItem item = result_set.GetRow(row);
				partition.id = atoi(item["id"]);
				partition.name = item["name"];
				partition.status = atoi(item["status"]);
				partition.is_recommend = atoi(item["recommend"]) != 0;
				partition.createtime = item["createtime"];
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
			else if (dynamic_cast<svr::DBAgentErrorRsp*>(message) != nullptr)
			{
				auto error = static_cast<svr::DBAgentErrorRsp*>(message);
				logger()->critical("查询分区信息失败，error code: {}", error->error_code());
			}
			assert(false);
			exit(-1);
		}
	};
	GlobalDBClient()->AsyncSelect(db::kMySQL, ServerConfig::GetInstance()->GetVerifyDBName(), "SELECT * FROM `partition`;", callback);
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
	try
	{
		std::unique_ptr<db::DBClient> db_client;
		asio::ip::tcp::endpoint endpoint(asio::ip::address_v4::from_string(ServerConfig::GetInstance()->GetDBAgentIP()),
			ServerConfig::GetInstance()->GetDBAgentPort());
		db_client = std::make_unique<db::DBClient>(threads, endpoint, ServerConfig::GetInstance()->GetDBAgentConnections());
		OnceInitGlobalDBClient(std::move(db_client));
	}
	catch (...)
	{
		logger()->critical("连接数据库代理服务器失败！");
		assert(false);
		exit(-1);
	}
	QueryPartitionInfo();
	threads.Run();

	return 0;
}
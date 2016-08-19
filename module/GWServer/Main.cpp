#include <iostream>
#include <common/Path.h>
#include <common/StringHelper.h>
#include "Logging.h"
#include "ServerConfig.h"
#include "SessionHandle.h"
#include "GatewayManager.h"

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
	std::string fullpath = path::curdir() + path::sep + "confgi.Router.json";
	if (!ServerConfig::GetInstance()->Load(fullpath))
	{
		logger()->critical("加载服务器配置失败！");
		assert(false);
		exit(-1);
	}
	logger()->info("加载服务器配置成功!");
	ServerConfig::GetInstance()->ProcessName(path::basename(*argv));

	// 启动网关服务器
	unsigned short use_thread_num = ServerConfig::GetInstance()->GetUseThreadNum();
	use_thread_num = use_thread_num == 0 ? std::thread::hardware_concurrency() : use_thread_num;
	asio::ip::tcp::endpoint endpoint(asio::ip::address_v4(), ServerConfig::GetInstance()->GetPort());
	network::IOServiceThreadManager threads(use_thread_num);

	GatewayManager gateway(threads);
	network::TCPServer server(endpoint, threads, std::bind(CreateSessionHandle, std::ref(gateway)),
		CreateMessageFilter, ServerConfig::GetInstance()->GetHeartbeatInterval());
	logger()->info("网关服务器[ip:{} port:{}]启动成功!", server.LocalEndpoint().address().to_string().c_str(), server.LocalEndpoint().port());
	threads.Run();

	return 0;
}
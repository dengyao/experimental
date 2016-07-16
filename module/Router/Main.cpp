#include <iostream>
#include <common/Path.h>
#include <common/StringHelper.h>
#include "ServerConfig.h"
#include "SessionHandle.h"
#include "RouterManager.h"

int main(int argc, char *argv[])
{
	// 加载服务器配置
	std::string fullpath = path::curdir() + path::sep + "confgi.Router.json";
	if (!ServerConfig::GetInstance()->Load(fullpath))
	{
		std::cout << "加载服务器配置失败！" << std::endl;
		assert(false);
		exit(-1);
	}

	// 启动路由器
	unsigned short use_thread_num = ServerConfig::GetInstance()->GetUseThreadNum();
	use_thread_num = use_thread_num == 0 ? std::thread::hardware_concurrency() : use_thread_num;
	asio::ip::tcp::endpoint endpoint(asio::ip::address_v4(), ServerConfig::GetInstance()->GetPort());
	network::IOServiceThreadManager threads(use_thread_num);
	threads.SetSessionTimeout(ServerConfig::GetInstance()->GetHeartbeatInterval());

	RouterManager router(threads);
	network::TCPServer server(endpoint, threads, std::bind(CreateSessionHandle, std::ref(router)), CreateMessageFilter);
	std::cout << string_helper::format("路由器[ip:%s port:%u]启动成功!", server.LocalEndpoint().address().to_string().c_str(), server.LocalEndpoint().port()) << std::endl;
	threads.Run();

	return 0;
}
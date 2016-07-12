#include <iostream>
#include "SessionHandle.h"
#include "GatewayManager.h"

int main(int argc, char *argv[])
{
	asio::ip::tcp::endpoint endpoint(asio::ip::address_v4(), 4235);
	eddy::IOServiceThreadManager threads(std::thread::hardware_concurrency());
	GatewayManager manager(threads);
	eddy::TCPServer server(endpoint, threads, std::bind(CreateSessionHandle, std::ref(manager)), CreateMessageFilter);
	threads.Run();

	return 0;
}
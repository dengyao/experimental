#include <iostream>
#include "SessionHandle.h"
#include "RouterManager.h"

int main(int argc, char *argv[])
{
	asio::ip::tcp::endpoint endpoint(asio::ip::address_v4(), 4235);
	network::IOServiceThreadManager threads(std::thread::hardware_concurrency());
	RouterManager manager(threads);
	network::TCPServer server(endpoint, threads, std::bind(CreateSessionHandle, std::ref(manager)), CreateMessageFilter);
	threads.Run();

	return 0;
}
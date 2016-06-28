#include <iostream>
#include "eddy.h"
#include "proto/db.request.pb.h"


int main(int argc, char *argv[])
{
	asio::error_code error_code;
	eddy::IOServiceThreadManager threads(1);
	asio::ip::tcp::endpoint endpoint(asio::ip::address_v4::from_string("192.168.1.109"), 4235);
	eddy::TCPClient client(threads, CreateSessionHandle, CreateMessageFilter);
	client.Connect(endpoint, error_code);
	if (error_code)
	{
		std::cerr << error_code.message() << std::endl;
		getchar();
	}
	threads.Run();
	
	return 0;
}
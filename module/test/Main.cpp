#include <iostream>
#include "eddy.h"


class SessionHandle : public eddy::TCPSessionHandle
{
	virtual void on_connect() override
	{
		std::cout << "on_connect" << std::endl;
	}

	virtual void on_message(eddy::NetMessage &message) override
	{
		std::string s(message.data(), message.readable());
		std::cout << "on_message:" << s << std::endl;
		send(message);
	}

	virtual void on_close() override
	{
		std::cout << "on_close" << std::endl;
	}
};

std::shared_ptr<SessionHandle> CreateSessionHandle()
{
	return std::make_shared<SessionHandle>();
}

std::shared_ptr<eddy::SimpleMessageFilter> CreateMessageFilter()
{
	return std::make_shared<eddy::SimpleMessageFilter>();
}

int main(int argc, char **argv)
{
	asio::ip::tcp::endpoint ep(asio::ip::address_v4(), 4235);
	eddy::IOServiceThreadManager thread_manager(std::thread::hardware_concurrency());
	eddy::TCPServer server(ep, thread_manager, CreateSessionHandle, CreateMessageFilter);
	thread_manager.run();

	return 0;
}
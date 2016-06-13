#include <iostream>
#include "eddy.h"


class SessionHandle : public eddy::TCPSessionHandle
{
	virtual void OnConnect() override
	{
		std::cout << "OnConnect" << std::endl;
	}

	virtual void OnMessage(eddy::NetMessage &message) override
	{
		std::string s(message.Data(), message.Readable());
		std::cout << "OnMessage:" << s << std::endl;
		Send(message);
	}

	virtual void OnClose() override
	{
		std::cout << "OnClose" << std::endl;
	}
};

std::shared_ptr<SessionHandle> CreateSessionHandle()
{
	return std::make_shared<SessionHandle>();
}

std::shared_ptr<eddy::DefaultMessageFilter> CreateMessageFilter()
{
	return std::make_shared<eddy::DefaultMessageFilter>();
}

int main(int argc, char **argv)
{
	asio::ip::tcp::endpoint ep(asio::ip::address_v4(), 4235);
	eddy::IOServiceThreadManager thread_manager(std::thread::hardware_concurrency());
	eddy::TCPServer server(ep, thread_manager, CreateSessionHandle, CreateMessageFilter, 8);
	thread_manager.Run();

	return 0;
}
#include <iostream>
#include "eddy.h"
#include "proto/db.request.pb.h"

class SessionHandle : public eddy::TCPSessionHandle
{
public:
	virtual void OnConnect() override
	{
		proto_db::Request result;
		result.set_number(1);
		result.set_dbtype(proto_db::Request_DatabaseType_kMySQL);
		result.set_action(proto_db::Request_ActoinType_kSelect);
		result.set_dbname("sgs");
		result.set_statement("SELECT * FROM `actor`;");
		char buffer[65525] = { 0 };
		result.SerializeToArray(buffer, 65525);
		eddy::NetMessage message(result.ByteSize());
		message.Write(buffer, result.ByteSize());
		Send(message);
	}

	virtual void OnMessage(eddy::NetMessage &message) override
	{
	}

	virtual void OnClose() override
	{
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
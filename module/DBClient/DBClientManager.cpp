#include "DBClientManager.h"


DBClientManager::DBClientManager(eddy::IOServiceThreadManager &threads, const std::string &host, unsigned short port, size_t client_num)
	: next_client_index_(0)
	, client_num_(client_num)
{
	KeepConnectionNumber();
}

void DBClientManager::AsyncQuery(int dbtype, const char *dbname, const char *sql, const QueryCallBack &callback)
{
	if (client_lists_.size() < client_num_)
	{
		KeepConnectionNumber();

		// 错误
	}
	else
	{

	}
}

void DBClientManager::KeepConnectionNumber()
{
	eddy::TCPClient client(threads_, );
	for (size_t i = client_lists_.size(); i < client_num_; ++i)
	{
		client.AsyncConnect();
		// 打印日志
	}
}

void DBClientManager::OnConnected(DBClient *client)
{
	assert(std::find(client_lists_.begin(), client_lists_.end(), client) == client_lists_.end());
	if (std::find(client_lists_.begin(), client_lists_.end(), client) == client_lists_.end())
	{
		client_lists_.push_back(client);
	}
}

void DBClientManager::OnDisconnect(DBClient *client)
{
	auto found = std::find(client_lists_.begin(), client_lists_.end(), client);
	assert(found != client_lists_.end());
	if (found != client_lists_.end())
	{
		client_lists_.erase(found);
	}
	KeepConnectionNumber();
}
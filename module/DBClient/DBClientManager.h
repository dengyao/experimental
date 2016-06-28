#ifndef __DBCLIENT_MANAGER_H__
#define __DBCLIENT_MANAGER_H__

#include <vector>
#include <functional>
#include "eddy.h"

class DBClient;

namespace google
{
	namespace protobuf
	{
		class Message;
	}
}

class DBClientManager
{
public:
	typedef std::function<void(google::protobuf::Message*)> QueryCallBack;

public:
	DBClientManager(eddy::IOServiceThreadManager &threads, const std::string &host, unsigned short port, size_t client_num);

	void AsyncQuery(int dbtype, const char *dbname, const char *sql, const QueryCallBack &callback);

public:
	void KeepConnectionNumber();

	void OnConnected(DBClient *client);

	void OnDisconnect(DBClient *client);

private:
	const size_t                      client_num_;
	std::vector<DBClient*>            client_lists_;
	size_t                            next_client_index_;
	std::map<uint32_t, QueryCallBack> task_queue_;
	eddy::IOServiceThreadManager&     threads_;
};

#endif

#ifndef __DBCLIENT_MANAGER_H__
#define __DBCLIENT_MANAGER_H__

#include <set>
#include <map>
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

enum DatabaseType
{
	kRedis = 1,
	kMySQL = 2,
};

enum DatabaseActionType
{
	kSelect = 1,
	kInsert = 2,
	kUpdate = 3,
	kDelete = 4
};

class DBClientManager
{
public:
	typedef std::function<void(google::protobuf::Message*)> QueryCallBack;

public:
	DBClientManager(eddy::IOServiceThreadManager &threads, const std::string &host, unsigned short port, size_t client_num);

	static DBClientManager* GetInstance();

public:
	void AsyncSelect(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback);

	void AsyncInsert(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback);

	void AsyncUpdate(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback);

	void AsyncDelete(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback);

private:
	void KeepConnectionNumber();

	void OnConnected(DBClient *client);

	void OnDisconnect(DBClient *client);

	void OnMessage(eddy::NetMessage &message);

	void AsyncQuery(DatabaseType dbtype, const char *dbname, DatabaseActionType action, const char *statement, QueryCallBack &&callback);

private:
	const size_t                            client_num_;
	eddy::IOServiceThreadManager&           threads_;
	eddy::IDGenerator                       generator_;
	std::vector<DBClient*>                  client_lists_;
	size_t                                  next_client_index_;
	std::map<uint32_t, QueryCallBack>       ongoing_lists_;
	std::map<DBClient*, std::set<uint32_t>> assigned_lists_;
};

#endif

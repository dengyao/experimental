#ifndef __DBCLIENT_MANAGER_H__
#define __DBCLIENT_MANAGER_H__

#include <set>
#include <map>
#include <vector>
#include "eddy.h"

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

class ConnectDBSFailed : public std::runtime_error
{
public:
	ConnectDBSFailed(const char *message)
		: std::runtime_error(message)
	{
	}
};

class DBClient;
class DBClientHanle;

class DBClient
{
	friend class DBClientHanle;

public:
	typedef std::function<void(google::protobuf::Message*)> QueryCallBack;

public:
	DBClient(eddy::IOServiceThreadManager &threads, asio::ip::tcp::endpoint &endpoint, size_t client_num);

	~DBClient();

	static DBClient* GetInstance();

	static void DestroyInstance();

public:
	size_t GetKeepAliveConnectionNum() const;

	void AsyncSelect(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback);

	void AsyncInsert(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback);

	void AsyncUpdate(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback);

	void AsyncDelete(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback);

private:
	void OnInitComplete();

	void OnConnected(DBClientHanle *client);

	void OnDisconnect(DBClientHanle *client);

	void OnMessage(eddy::NetMessage &message);

private:
	void Clear();

	void InitConnections();

	void ConnectionKeepAlive();

	eddy::SessionHandlePointer CreateClient();

	void AsyncQuery(DatabaseType dbtype, const char *dbname, DatabaseActionType action, const char *statement, QueryCallBack &&callback);

private:
	const size_t                                  client_num_;
	eddy::IOServiceThreadManager&                 threads_;
	eddy::IDGenerator                             generator_;
	std::set<std::shared_ptr<bool> >              lifetimes_;
	eddy::TCPClient					              client_creator_;
	std::vector<DBClientHanle*>                   client_lists_;
	std::map<uint32_t, QueryCallBack>             ongoing_lists_;
	std::map<DBClientHanle*, std::set<uint32_t> > assigned_lists_;
	asio::ip::tcp::endpoint                       endpoint_;
	size_t                                        next_client_index_;
};

#endif

#ifndef __DBCLIENT_MANAGER_H__
#define __DBCLIENT_MANAGER_H__

#include <set>
#include <map>
#include <vector>
#include <eddy.h>

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

class DBClientHandle;

class DBClient final
{
	friend class DBClientHandle;

public:
	typedef std::function<void(google::protobuf::Message*)> QueryCallBack;

public:
	DBClient(eddy::IOServiceThreadManager &threads, asio::ip::tcp::endpoint &endpoint, size_t connection_num = 0);

	~DBClient();

public:
	// 执行登录
	bool Login(std::chrono::seconds timeout);

	// 获取有效连接数量
	size_t GetKeepAliveConnectionNum() const;

	// 异步查询
	void AsyncSelect(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback);

	// 异步插入
	void AsyncInsert(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback);

	// 异步更新
	void AsyncUpdate(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback);

	// 异步删除
	void AsyncDelete(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback);

private:
	// 连接事件
	void OnConnected(DBClientHandle *client);

	// 断开连接事件
	void OnDisconnect(DBClientHandle *client);

	// 接受消息事件
	void OnMessage(DBClientHandle *client, google::protobuf::Message *message);

private:
	// 清理所有连接
	void Clear();

	// 初始化连接
	void InitConnections();

	// 创建会话处理器
	eddy::SessionHandlePointer CreateClientHandle();

	// 异步操作
	void AsyncQuery(DatabaseType dbtype, const char *dbname, DatabaseActionType action, const char *statement, QueryCallBack &&callback);

private:
	const size_t                                   connection_num_;
	eddy::IOServiceThreadManager&                  threads_;
	eddy::IDGenerator                              generator_;
	std::set<std::shared_ptr<bool> >               lifetimes_;
	eddy::TCPClient					               client_creator_;
	std::vector<DBClientHandle*>                   client_lists_;
	std::map<uint32_t, QueryCallBack>              ongoing_lists_;
	std::map<DBClientHandle*, std::set<uint32_t> > assigned_lists_;
	asio::ip::tcp::endpoint                        endpoint_;
	size_t                                         next_client_index_;
};

#endif

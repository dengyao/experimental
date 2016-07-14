#ifndef __DBCLIENT_MANAGER_H__
#define __DBCLIENT_MANAGER_H__

#include <set>
#include <map>
#include <vector>
#include <network.h>

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

class DBSessionHandle;
class AsyncReconnectHandle;

class DBClient final
{
	friend class DBSessionHandle;
	friend class AsyncReconnectHandle;

public:
	typedef std::function<void(google::protobuf::Message*)> QueryCallBack;

public:
	DBClient(network::IOServiceThreadManager &threads, asio::ip::tcp::endpoint &endpoint, size_t connection_num);
	~DBClient();

public:
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
	void OnConnected(DBSessionHandle *session);

	// 断开连接事件
	void OnDisconnect(DBSessionHandle *session);

	// 接受消息事件
	void OnMessage(DBSessionHandle *session, google::protobuf::Message *message);

private:
	// 清理所有连接
	void Clear();

	// 初始化连接
	void InitConnections();

	// 异步重连
	void AsyncReconnect();

	// 异步重连结果
	void AsyncReconnectResult(AsyncReconnectHandle &handler, asio::error_code error_code);

	// 更新计时器
	void UpdateTimer(asio::error_code error_code);

	// 创建会话处理器
	network::SessionHandlePointer CreateSessionHandle();

	// 异步操作
	void AsyncQuery(DatabaseType dbtype, const char *dbname, DatabaseActionType action, const char *statement, QueryCallBack &&callback);

private:
	const size_t                                    connection_num_;
	unsigned short                                  connecting_num_;
	network::IOServiceThreadManager&                threads_;
	network::IDGenerator                            generator_;
	std::set<std::shared_ptr<bool> >                lifetimes_;
	network::TCPClient					            session_handle_creator_;
	std::vector<DBSessionHandle*>                   session_handle_lists_;
	std::map<uint32_t, QueryCallBack>               ongoing_lists_;
	std::map<DBSessionHandle*, std::set<uint32_t> > assigned_lists_;
	asio::ip::tcp::endpoint                         endpoint_;
	asio::steady_timer                              timer_;
	const std::function<void(asio::error_code)>     wait_handler_;
	size_t                                          next_client_index_;
};

#endif

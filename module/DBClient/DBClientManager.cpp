#include "DBClientManager.h"
#include "proto/db.request.pb.h"
#include "proto/db.response.pb.h"

static DBClientManager *g_db_client_manager = nullptr;

DBClientManager::DBClientManager(eddy::IOServiceThreadManager &threads, const std::string &host, unsigned short port, size_t client_num)
    : threads_(threads)
	, generator_(65535)
	, next_client_index_(0)
	, client_num_(client_num)
{
	KeepConnectionNumber();
	assert(g_db_client_manager == nullptr);
	g_db_client_manager = this;
}

DBClientManager* DBClientManager::GetInstance()
{
	return g_db_client_manager;
}

void DBClientManager::AsyncQuery(DatabaseType dbtype, const char *dbname, DatabaseActionType action, const char *statement, QueryCallBack &&callback)
{
	static_assert(DatabaseType::kRedis == proto_db::Request::kRedis &&
		DatabaseType::kMySQL == proto_db::Request::kMySQL, "type mismatch");

	static_assert(DatabaseActionType::kSelect == proto_db::Request::kSelect &&
		DatabaseActionType::kInsert == proto_db::Request::kInsert &&
		DatabaseActionType::kUpdate == proto_db::Request::kUpdate &&
		DatabaseActionType::kDelete == proto_db::Request::kDelete, "type mismatch");

	if (client_lists_.size() < client_num_)
	{
		KeepConnectionNumber();
		if (client_lists_.empty())
		{
			proto_db::ProxyError error;
			error.set_identifier(0);
			error.set_error_code(proto_db::ProxyError::kNotConnected);
			callback(&error);
			return;
		}
	}

	uint32_t identifier = 0;
	if (generator_.Get(identifier))
	{
		if (next_client_index_ >= client_lists_.size())
		{
			next_client_index_ = 0;
		}

		proto_db::Request request;
		request.set_dbname(dbname);
		request.set_statement(statement);
		request.set_identifier(identifier);
		request.set_action(static_cast<proto_db::Request::ActoinType>(action));
		request.set_dbtype(static_cast<proto_db::Request::DatabaseType>(dbtype));

		eddy::NetMessage message;
		int size = request.ByteSize();
		message.EnsureWritableBytes(size);
		request.SerializeToArray(message.Data(), size);

		DBClient *client = client_lists_[next_client_index_++];
		assigned_lists_[client].insert(identifier);
		ongoing_lists_.insert(std::make_pair(identifier, std::forward<QueryCallBack>(callback)));
		
		//client->Send(message);
	}
}

void DBClientManager::AsyncSelect(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback)
{
	AsyncQuery(dbtype, dbname, DatabaseActionType::kSelect, statement, std::forward<QueryCallBack>(callback));
}

void DBClientManager::AsyncInsert(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback)
{
	AsyncQuery(dbtype, dbname, DatabaseActionType::kInsert, statement, std::forward<QueryCallBack>(callback));
}

void DBClientManager::AsyncUpdate(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback)
{
	AsyncQuery(dbtype, dbname, DatabaseActionType::kUpdate, statement, std::forward<QueryCallBack>(callback));
}

void DBClientManager::AsyncDelete(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback)
{
	AsyncQuery(dbtype, dbname, DatabaseActionType::kDelete, statement, std::forward<QueryCallBack>(callback));
}

void DBClientManager::KeepConnectionNumber()
{
	eddy::TCPClient *client;
	for (size_t i = client_lists_.size(); i < client_num_; ++i)
	{
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
	auto client_iter = std::find(client_lists_.begin(), client_lists_.end(), client);
	assert(client_iter != client_lists_.end());
	if (client_iter != client_lists_.end())
	{
		client_lists_.erase(client_iter);
	}

	auto found = assigned_lists_.find(client);
	if (found != assigned_lists_.end())
	{
		std::set<uint32_t> &lists = found->second;
		for (auto iter = lists.begin(); iter != lists.end(); ++iter)
		{
			generator_.Put(*iter);
			auto callback_iter = ongoing_lists_.find(*iter);
			assert(callback_iter != ongoing_lists_.end());
			if (callback_iter != ongoing_lists_.end())
			{
				QueryCallBack func = std::move(callback_iter->second);
				ongoing_lists_.erase(callback_iter);

				proto_db::ProxyError error;
				error.set_identifier(0);
				error.set_error_code(proto_db::ProxyError::kDisconnect);
				func(&error);
			}
		}
	}

	KeepConnectionNumber();
}

void DBClientManager::OnMessage(eddy::NetMessage &message)
{

}
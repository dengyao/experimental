#include "DBClient.h"
#include <limits>
#include <iostream>
#include "proto/db.request.pb.h"
#include "proto/db.response.pb.h"


static DBClientPointer g_client_instance;

class DBClientHanle : public eddy::TCPSessionHandler
{
	friend class DBClient;

public:
	DBClientHanle(DBClientPointer &client)
		: client_(client)
	{
	}

	virtual void OnConnect() override
	{
		if (!client_.expired)
		{
			client_.lock()->OnConnected(this);
		}
	}

	virtual void OnMessage(eddy::NetMessage &message) override
	{
		if (!client_.expired)
		{
			client_.lock()->OnMessage(message);
		}
	}

	virtual void OnClose() override
	{
		if (!client_.expired)
		{
			client_.lock()->OnDisconnect(this);
		}
	}

private:
	std::weak_ptr<DBClient> client_;
};

eddy::MessageFilterPointer CreaterMessageFilter()
{
	return std::make_shared<eddy::DefaultMessageFilter>();
}

DBClient::DBClient(eddy::IOServiceThreadManager &threads, asio::ip::tcp::endpoint &endpoint, size_t client_num)
	: threads_(threads)
	, endpoint_(endpoint)
	, next_client_index_(0)
	, client_num_(client_num)
	, generator_(std::numeric_limits<uint16_t>::max())
	, client_creator_(threads_, std::bind(&DBClient::CreateClient, this), std::bind(CreaterMessageFilter))
{
	InitConnections();
	assert(g_client_instance == nullptr);
}

DBClient::~DBClient()
{
}

const DBClientPointer& DBClient::GetInstance()
{
	return g_client_instance;
}

void DBClient::DestroyInstance()
{
	g_client_instance = nullptr;
}

void DBClient::OnInitComplete()
{
	g_client_instance = shared_from_this();
}

eddy::SessionHandlePointer DBClient::CreateClient()
{
	return std::make_shared<DBClientHanle>(shared_from_this());
}

void DBClient::InitConnections()
{
	asio::error_code error_code;
	for (size_t i = 0; i < client_num_; ++i)
	{
		eddy::TCPSessionID id = client_creator_.Connect(endpoint_, error_code);
		if (error_code)
		{
			throw ConnectDBSFailed(error_code.message().c_str());
		}
		SessionHandlePointer handle = threads_.SessionHandler(id);
	}
}

void DBClient::ConnectionKeepAlive()
{
	for (size_t i = client_lists_.size(); i < client_num_; ++i)
	{
		client_creator_.AsyncConnect(endpoint_);
	}
}

void DBClient::OnConnected(DBClientHanle *client)
{
	if (std::find(client_lists_.begin(), client_lists_.end(), client) == client_lists_.end())
	{
		client_lists_.push_back(client);
	}
}

void DBClient::OnDisconnect(DBClientHanle *client)
{
	auto client_iter = std::find(client_lists_.begin(), client_lists_.end(), client);
	if (client_iter != client_lists_.end())
	{
		client_lists_.erase(client_iter);
		ConnectionKeepAlive();
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
}

void DBClient::OnMessage(eddy::NetMessage &message)
{

}

size_t DBClient::GetKeepAliveConnectionNum() const
{
	return client_lists_.size();
}

void DBClient::AsyncQuery(DatabaseType dbtype, const char *dbname, DatabaseActionType action, const char *statement, QueryCallBack &&callback)
{
	static_assert(DatabaseType::kRedis == proto_db::Request::kRedis &&
		DatabaseType::kMySQL == proto_db::Request::kMySQL, "type mismatch");

	static_assert(DatabaseActionType::kSelect == proto_db::Request::kSelect &&
		DatabaseActionType::kInsert == proto_db::Request::kInsert &&
		DatabaseActionType::kUpdate == proto_db::Request::kUpdate &&
		DatabaseActionType::kDelete == proto_db::Request::kDelete, "type mismatch");

	if (client_lists_.size() < client_num_)
	{
		ConnectionKeepAlive();
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

		DBClientHanle *client = client_lists_[next_client_index_++];
		assigned_lists_[client].insert(identifier);
		ongoing_lists_.insert(std::make_pair(identifier, std::forward<QueryCallBack>(callback)));
		
		client->Send(message);
	}
}

void DBClient::AsyncSelect(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback)
{
	AsyncQuery(dbtype, dbname, DatabaseActionType::kSelect, statement, std::forward<QueryCallBack>(callback));
}

void DBClient::AsyncInsert(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback)
{
	AsyncQuery(dbtype, dbname, DatabaseActionType::kInsert, statement, std::forward<QueryCallBack>(callback));
}

void DBClient::AsyncUpdate(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback)
{
	AsyncQuery(dbtype, dbname, DatabaseActionType::kUpdate, statement, std::forward<QueryCallBack>(callback));
}

void DBClient::AsyncDelete(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback)
{
	AsyncQuery(dbtype, dbname, DatabaseActionType::kDelete, statement, std::forward<QueryCallBack>(callback));
}
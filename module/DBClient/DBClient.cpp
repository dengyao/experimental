#include "DBClient.h"
#include <limits>
#include <iostream>
#include <proto/MessageHelper.h>
#include <proto/internal.protocol.pb.h>

class DBClientHandle : public eddy::TCPSessionHandler
{
	friend class DBClient;

public:
	DBClientHandle(DBClient *client, std::shared_ptr<bool> &shared)
		: client_(client)
		, is_logged_(false)
		, client_shared_(shared)
	{
	}

	std::shared_ptr<bool>& GetShared()
	{
		return client_shared_;
	}

	// 连接成功
	virtual void OnConnect() override
	{
		if (!is_logged_)
		{
			// 发送登录请求
			eddy::NetMessage message;
			internal::LoginDBProxyReq login;
			PackageMessage(&login, message);
			Send(message);
		}
	}

	// 接收消息
	virtual void OnMessage(eddy::NetMessage &message) override
	{
		auto respond = UnpackageMessage(message);
		if (respond == nullptr)
		{
			assert(false);
			return;
		}

		if (!is_logged_)
		{
			// 是否登录成功
			if (dynamic_cast<internal::PongRsp*>(respond.get()) == nullptr)
			{
				if (dynamic_cast<internal::LoginDBProxyRsp*>(respond.get()) == nullptr)
				{
					assert(false);
					return;
				}
				is_logged_ = true;
				if (!client_shared_.unique())
				{
					client_->OnConnected(this);
				}
			}
		}
		else if (dynamic_cast<internal::PongRsp*>(respond.get()) == nullptr)
		{
			if (!client_shared_.unique())
			{
				client_->OnMessage(this, respond.get());
			}
		}
	}

	// 连接关闭
	virtual void OnClose() override
	{
		if (is_logged_)
		{
			is_logged_ = false;
			if (!client_shared_.unique())
			{
				client_->OnDisconnect(this);
			}
		}
	}

private:
	DBClient*             client_;
	bool                  is_logged_;
	std::shared_ptr<bool> client_shared_;
};

eddy::MessageFilterPointer CreaterMessageFilter()
{
	return std::make_shared<eddy::DefaultMessageFilter>();
}

DBClient::DBClient(eddy::IOServiceThreadManager &threads, asio::ip::tcp::endpoint &endpoint, size_t connection_num)
	: threads_(threads)
	, endpoint_(endpoint)
	, next_client_index_(0)
	, connection_num_(connection_num)
	, generator_(std::numeric_limits<uint16_t>::max())
	, client_creator_(threads_, std::bind(&DBClient::CreateClientHandle, this), std::bind(CreaterMessageFilter))
{
	InitConnections();
}

DBClient::~DBClient()
{
	Clear();
}

// 创建会话处理器
eddy::SessionHandlePointer DBClient::CreateClientHandle()
{
	auto life = std::make_shared<bool>();
	lifetimes_.insert(life);
	return std::make_shared<DBClientHandle>(this, life);
}

// 清理所有连接
void DBClient::Clear()
{
	lifetimes_.clear();
	for (auto handle : client_lists_)
	{
		if (handle != nullptr)
		{
			handle->Close();
		}
	}
	client_lists_.clear();
}

// 初始化连接
void DBClient::InitConnections()
{
	asio::error_code error_code;
	for (size_t i = 0; i < connection_num_; ++i)
	{
		eddy::TCPSessionID id = client_creator_.Connect(endpoint_, error_code);
		if (error_code)
		{
			Clear();
			throw ConnectDBSFailed(error_code.message().c_str());
		}
	}
}

// 连接事件
void DBClient::OnConnected(DBClientHandle *client)
{
	if (client_lists_.size() >= connection_num_)
	{
		assert(false);
		client->Close();
		return;
	}

	if (std::find(client_lists_.begin(), client_lists_.end(), client) == client_lists_.end())
	{
		client_lists_.push_back(client);
	}
}

// 断开连接事件
void DBClient::OnDisconnect(DBClientHandle *client)
{
	lifetimes_.erase(client->GetShared());

	auto client_iter = std::find(client_lists_.begin(), client_lists_.end(), client);
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
				QueryCallBack callback = std::move(callback_iter->second);
				ongoing_lists_.erase(callback_iter);

				if (callback != nullptr)
				{
					internal::DBProxyErrorRsp error;
					error.set_sequence(0);
					error.set_error_code(internal::DBProxyErrorRsp::kDisconnect);
					callback(&error);
				}	
			}
		}
		assigned_lists_.erase(found);
	}
}

// 接受消息事件
void DBClient::OnMessage(DBClientHandle *client, google::protobuf::Message *message)
{
	uint32_t sequence = 0;
	if (dynamic_cast<internal::QueryDBProxyRsp*>(message) != nullptr)
	{
		sequence = dynamic_cast<internal::QueryDBProxyRsp*>(message)->sequence();
	}
	else if (dynamic_cast<internal::DBErrorRsp*>(message) != nullptr)
	{
		sequence = dynamic_cast<internal::DBErrorRsp*>(message)->sequence();
	}
	else if (dynamic_cast<internal::DBProxyErrorRsp*>(message) != nullptr)
	{
		sequence = dynamic_cast<internal::DBProxyErrorRsp*>(message)->sequence();
	}
	else
	{
		assert(false);
		return;
	}

	auto set_iter = assigned_lists_.find(client);
	if (set_iter != assigned_lists_.end())
	{
		set_iter->second.erase(sequence);
	}

	QueryCallBack callback;
	auto callback_iter = ongoing_lists_.find(sequence);
	if (callback_iter != ongoing_lists_.end())
	{
		callback = std::move(callback_iter->second);
		ongoing_lists_.erase(callback_iter);
	}

	if (sequence > 0)
	{
		generator_.Put(sequence);
	}

	if (callback != nullptr)
	{
		callback(message);
	}
}

// 执行登录
bool DBClient::Login(std::chrono::seconds timeout)
{
	while (connection_num_ != client_lists_.size())
	{
		std::this_thread::yield();
	}
	return false;
}

// 获取有效连接数量
size_t DBClient::GetKeepAliveConnectionNum() const
{
	return client_lists_.size();
}

// 异步操作
void DBClient::AsyncQuery(DatabaseType dbtype, const char *dbname, DatabaseActionType action, const char *statement, QueryCallBack &&callback)
{
	static_assert(DatabaseType::kRedis == internal::QueryDBProxyReq::kRedis &&
		DatabaseType::kMySQL == internal::QueryDBProxyReq::kMySQL, "type mismatch");

	static_assert(DatabaseActionType::kSelect == internal::QueryDBProxyReq::kSelect &&
		DatabaseActionType::kInsert == internal::QueryDBProxyReq::kInsert &&
		DatabaseActionType::kUpdate == internal::QueryDBProxyReq::kUpdate &&
		DatabaseActionType::kDelete == internal::QueryDBProxyReq::kDelete, "type mismatch");

	if (client_lists_.size() < connection_num_)
	{
		if (client_lists_.empty())
		{
			internal::DBProxyErrorRsp error;
			error.set_sequence(0);
			error.set_error_code(internal::DBProxyErrorRsp::kNotConnected);
			callback(&error);
			return;
		}
	}

	uint32_t sequence = 0;
	if (generator_.Get(sequence))
	{
		if (next_client_index_ >= client_lists_.size())
		{
			next_client_index_ = 0;
		}

		internal::QueryDBProxyReq request;
		request.set_dbname(dbname);
		request.set_statement(statement);
		request.set_sequence(sequence);
		request.set_action(static_cast<internal::QueryDBProxyReq::ActoinType>(action));
		request.set_dbtype(static_cast<internal::QueryDBProxyReq::DatabaseType>(dbtype));

		DBClientHandle *client = client_lists_[next_client_index_++];
		assigned_lists_[client].insert(sequence);
		ongoing_lists_.insert(std::make_pair(sequence, std::forward<QueryCallBack>(callback)));
		
		eddy::NetMessage message;
		PackageMessage(&request, message);
		client->Send(message);
	}
}

// 异步查询
void DBClient::AsyncSelect(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback)
{
	AsyncQuery(dbtype, dbname, DatabaseActionType::kSelect, statement, std::forward<QueryCallBack>(callback));
}

// 异步插入
void DBClient::AsyncInsert(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback)
{
	AsyncQuery(dbtype, dbname, DatabaseActionType::kInsert, statement, std::forward<QueryCallBack>(callback));
}

// 异步更新
void DBClient::AsyncUpdate(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback)
{
	AsyncQuery(dbtype, dbname, DatabaseActionType::kUpdate, statement, std::forward<QueryCallBack>(callback));
}

// 异步删除
void DBClient::AsyncDelete(DatabaseType dbtype, const char *dbname, const char *statement, QueryCallBack &&callback)
{
	AsyncQuery(dbtype, dbname, DatabaseActionType::kDelete, statement, std::forward<QueryCallBack>(callback));
}
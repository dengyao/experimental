#include "DBClient.h"
#include <limits>
#include <iostream>
#include <proto/MessageHelper.h>
#include <proto/internal.protocol.pb.h>

/************************************************************************/

class DBClientHandle : public network::TCPSessionHandler
{
	friend class DBClient;

public:
	DBClientHandle(DBClient *client, std::shared_ptr<bool> &life)
		: counter_(0)
		, client_(client)
		, is_logged_(false)
		, client_life_(life)
		, heartbeat_interval_(0)
	{
	}

	// 获取计数器
	std::shared_ptr<bool>& GetShared()
	{
		return client_life_;
	}

	// 连接成功
	virtual void OnConnect() override
	{
		network::NetMessage message;
		internal::LoginDBAgentReq login;
		PackageMessage(&login, message);
		Send(message);

		if (!client_life_.unique())
		{
			client_->OnConnected(this);
		}
	}

	// 接收消息
	virtual void OnMessage(network::NetMessage &message) override
	{
		auto respond = UnpackageMessage(message);
		if (respond == nullptr)
		{
			assert(false);
			return;
		}

		if (!is_logged_)
		{
			if (dynamic_cast<internal::PongRsp*>(respond.get()) == nullptr)
			{
				if (dynamic_cast<internal::LoginDBAgentRsp*>(respond.get()) != nullptr)
				{
					is_logged_ = true;
					counter_ = heartbeat_interval_ = dynamic_cast<internal::LoginDBAgentRsp*>(respond.get())->heartbeat_interval();
				}
				else
				{
					assert(false);
					return;
				}
			}
		}
		else if (dynamic_cast<internal::PongRsp*>(respond.get()) == nullptr)
		{
			if (!client_life_.unique())
			{
				client_->OnMessage(this, respond.get());
			}
		}
		else
		{
			assert(false);
		}
	}

	// 连接关闭
	virtual void OnClose() override
	{
		is_logged_ = false;
		if (!client_life_.unique())
		{
			client_->OnDisconnect(this);
		}
	}

	// 发送心跳倒计时
	void HeartbeatCountdown()
	{
		if (is_logged_ && heartbeat_interval_ > 0)
		{
			if (--counter_ == 0)
			{
				internal::PingReq req;
				network::NetMessage message;
				PackageMessage(&req, message);
				Send(message);
				counter_ = heartbeat_interval_;
			}		
		}	
	}

private:
	DBClient*             client_;
	bool                  is_logged_;
	std::shared_ptr<bool> client_life_;
	uint32_t              counter_;
	uint32_t              heartbeat_interval_;
};

/************************************************************************/
/************************************************************************/

class AsyncReconnectHandle : public std::enable_shared_from_this< AsyncReconnectHandle >
{
public:
	AsyncReconnectHandle(DBClient *client, std::shared_ptr<bool> &life)
		: client_(client)
		, client_life_(life)
	{
	}

	// 获取计数器
	std::shared_ptr<bool>& GetShared()
	{
		return client_life_;
	}

	// 连接结果回调
	void ConnectCallback(asio::error_code error_code)
	{
		if (!client_life_.unique())
		{
			client_->AsyncReconnectResult(*this, error_code);
		}
	}

private:
	DBClient*             client_;
	std::shared_ptr<bool> client_life_;
};

/************************************************************************/
/************************************************************************/

network::MessageFilterPointer CreaterMessageFilter()
{
	return std::make_shared<network::DefaultMessageFilter>();
}

DBClient::DBClient(network::IOServiceThreadManager &threads, asio::ip::tcp::endpoint &endpoint, size_t connection_num)
	: threads_(threads)
	, connecting_num_(0)
	, endpoint_(endpoint)
	, next_client_index_(0)
	, connection_num_(connection_num)
	, timer_(threads.MainThread()->IOService())
	, generator_(std::numeric_limits<uint16_t>::max())
	, wait_handler_(std::bind(&DBClient::UpdateTimer, this, std::placeholders::_1))
	, client_creator_(threads_, std::bind(&DBClient::CreateClientHandle, this), std::bind(CreaterMessageFilter))
{
	InitConnections();
	timer_.expires_from_now(std::chrono::seconds(1));
	timer_.async_wait(wait_handler_);
}

DBClient::~DBClient()
{
	Clear();
}

// 创建会话处理器
network::SessionHandlePointer DBClient::CreateClientHandle()
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
		try
		{
			client_creator_.Connect(endpoint_, error_code);

		}
		catch (const std::exception&)
		{
			Clear();
			throw ConnectDBSFailed(error_code.message().c_str());
		}
		
		if (error_code)
		{
			Clear();
			throw ConnectDBSFailed(error_code.message().c_str());
		}
	}
}

// 重新连接
void DBClient::AsyncReconnect()
{
	if (client_lists_.size() < connection_num_)
	{
		const int diff = connection_num_ - client_lists_.size();
		assert(connecting_num_ <= diff);
		if (connecting_num_ < diff)
		{
			const int lack = diff - connecting_num_;
			for (int i = 0; i < lack; ++i)
			{
				++connecting_num_;

				auto life = std::make_shared<bool>();
				lifetimes_.insert(life);

				auto handler = std::make_shared<AsyncReconnectHandle>(this, life);
				client_creator_.AsyncConnect(endpoint_, std::bind(&AsyncReconnectHandle::ConnectCallback, std::move(handler), std::placeholders::_1));
			}
		}
	}
}

// 异步重连结果
void DBClient::AsyncReconnectResult(AsyncReconnectHandle &handler, asio::error_code error_code)
{
	if (error_code)
	{
		assert(connecting_num_ > 0);
		if (connecting_num_ > 0)
		{
			--connecting_num_;
		}
	}
	else
	{
		++connecting_num_;
	}
	lifetimes_.erase(handler.GetShared());
}

// 更新计时器
void DBClient::UpdateTimer(asio::error_code error_code)
{
	if (error_code)
	{
		std::cerr << error_code.message() << std::endl;
		return;
	}

	for (auto handle : client_lists_)
	{
		handle->HeartbeatCountdown();
	}
	timer_.expires_from_now(std::chrono::seconds(1));
	timer_.async_wait(wait_handler_);
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
					internal::DBAgentErrorRsp error;
					error.set_sequence(0);
					error.set_error_code(internal::kDisconnect);
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
	if (dynamic_cast<internal::QueryDBAgentRsp*>(message) != nullptr)
	{
		sequence = dynamic_cast<internal::QueryDBAgentRsp*>(message)->sequence();
	}
	else if (dynamic_cast<internal::DBErrorRsp*>(message) != nullptr)
	{
		sequence = dynamic_cast<internal::DBErrorRsp*>(message)->sequence();
	}
	else if (dynamic_cast<internal::DBAgentErrorRsp*>(message) != nullptr)
	{
		sequence = dynamic_cast<internal::DBAgentErrorRsp*>(message)->sequence();
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

// 获取有效连接数量
size_t DBClient::GetKeepAliveConnectionNum() const
{
	return client_lists_.size();
}

// 异步操作
void DBClient::AsyncQuery(DatabaseType dbtype, const char *dbname, DatabaseActionType action, const char *statement, QueryCallBack &&callback)
{
	static_assert(DatabaseType::kRedis == internal::QueryDBAgentReq::kRedis &&
		DatabaseType::kMySQL == internal::QueryDBAgentReq::kMySQL, "type mismatch");

	static_assert(DatabaseActionType::kSelect == internal::QueryDBAgentReq::kSelect &&
		DatabaseActionType::kInsert == internal::QueryDBAgentReq::kInsert &&
		DatabaseActionType::kUpdate == internal::QueryDBAgentReq::kUpdate &&
		DatabaseActionType::kDelete == internal::QueryDBAgentReq::kDelete, "type mismatch");

	if (client_lists_.size() < connection_num_)
	{
		AsyncReconnect();

		if (client_lists_.empty())
		{
			internal::DBAgentErrorRsp error;
			error.set_sequence(0);
			error.set_error_code(internal::kNotConnected);
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

		internal::QueryDBAgentReq request;
		request.set_dbname(dbname);
		request.set_statement(statement);
		request.set_sequence(sequence);
		request.set_action(static_cast<internal::QueryDBAgentReq::ActoinType>(action));
		request.set_dbtype(static_cast<internal::QueryDBAgentReq::DatabaseType>(dbtype));

		DBClientHandle *client = client_lists_[next_client_index_++];
		assigned_lists_[client].insert(sequence);
		ongoing_lists_.insert(std::make_pair(sequence, std::forward<QueryCallBack>(callback)));
		
		network::NetMessage message;
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

/************************************************************************/
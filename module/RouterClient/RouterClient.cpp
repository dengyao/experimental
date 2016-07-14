#include "RouterClient.h"
#include <iostream>
#include <ProtobufCodec.h>
#include <proto/internal.pb.h>

/************************************************************************/

class RouterSessionHandle : public network::TCPSessionHandler
{
	friend class RouterClient;

public:
	RouterSessionHandle(RouterClient *client, std::shared_ptr<bool> &life)
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
		if (client_life_.unique())
		{
			Close();
		}
		else
		{
			network::NetMessage message;
			internal::LoginRouterReq request;
			request.set_child_id(client_->GetChildNodeID());
			request.set_type(static_cast<internal::NodeType>(client_->GetNodeType()));
			ProtubufCodec::Encode(&request, message);
			Send(message);
			client_->OnConnected(this);
		}
	}

	// 接收消息
	virtual void OnMessage(network::NetMessage &message) override
	{
		if (client_life_.unique())
		{
			Close();
			return;
		}

		auto response = ProtubufCodec::Decode(message);
		if (response == nullptr)
		{
			assert(false);
			return;
		}

		if (!is_logged_)
		{
			if (dynamic_cast<internal::PongRsp*>(response.get()) == nullptr)
			{
				if (dynamic_cast<internal::LoginRouterRsp*>(response.get()) != nullptr)
				{
					is_logged_ = true;
					counter_ = heartbeat_interval_ = static_cast<internal::LoginRouterRsp*>(response.get())->heartbeat_interval();
				}
				else
				{
					assert(false);
					return;
				}
			}
		}
		else if (dynamic_cast<internal::PongRsp*>(response.get()) == nullptr)
		{
			client_->OnMessage(this, response.get(), message);
		}
		else
		{
			assert(dynamic_cast<internal::PongRsp*>(response.get()) != nullptr);
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
				internal::PingReq request;
				network::NetMessage message;
				ProtubufCodec::Encode(&request, message);
				Send(message);
				counter_ = heartbeat_interval_;
			}
		}
	}

private:
	RouterClient*         client_;
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
	AsyncReconnectHandle(RouterClient *client, std::shared_ptr<bool> &life)
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
	RouterClient*         client_;
	std::shared_ptr<bool> client_life_;
};

/************************************************************************/
/************************************************************************/

network::MessageFilterPointer CreaterMessageFilter()
{
	return std::make_shared<network::DefaultMessageFilter>();
}

RouterClient::RouterClient(network::IOServiceThreadManager &threads, asio::ip::tcp::endpoint &endpoint, size_t connection_num, int node_type, int child_id)
	: threads_(threads)
	, connecting_num_(0)
	, endpoint_(endpoint)
	, child_id_(child_id)
	, node_type_(node_type)
	, next_client_index_(0)
	, connection_num_(connection_num)
	, timer_(threads.MainThread()->IOService(), std::chrono::seconds(1))
	, wait_handler_(std::bind(&RouterClient::OnUpdateTimer, this, std::placeholders::_1))
	, session_handle_creator_(threads_, std::bind(&RouterClient::CreateSessionHandle, this), std::bind(CreaterMessageFilter))
{
	assert(connection_num > 0);
	assert(node_type_ >= internal::NodeType_MIN && node_type_ <= internal::NodeType_MAX);
	InitConnections();
	timer_.async_wait(wait_handler_);
}

RouterClient::~RouterClient()
{
	Clear();
}

// 服务器节点类型
int RouterClient::GetNodeType() const
{
	return node_type_;
}

// 服务器子节点id
int RouterClient::GetChildNodeID() const
{
	return child_id_;
}

// 设置消息回调
void RouterClient::SetMessageCallback(const Callback &cb)
{
	message_cb_ = cb;
}

// 清理所有连接
void RouterClient::Clear()
{
	lifetimes_.clear();
	for (auto session : session_handle_lists_)
	{
		if (session != nullptr)
		{
			session->Close();
		}
	}
	session_handle_lists_.clear();
}

// 创建会话处理器
network::SessionHandlePointer RouterClient::CreateSessionHandle()
{
	auto life = std::make_shared<bool>();
	lifetimes_.insert(life);
	return std::make_shared<RouterSessionHandle>(this, life);
}

// 初始化连接
void RouterClient::InitConnections()
{
	asio::error_code error_code;
	for (size_t i = 0; i < connection_num_; ++i)
	{
		try
		{
			session_handle_creator_.Connect(endpoint_, error_code);
		}
		catch (const std::exception&)
		{
			Clear();
			throw ConnectRouterFailed(error_code.message().c_str());
		}

		if (error_code)
		{
			Clear();
			throw ConnectRouterFailed(error_code.message().c_str());
		}
	}
}

// 重新连接
void RouterClient::AsyncReconnect()
{
	if (session_handle_lists_.size() < connection_num_)
	{
		const int diff = connection_num_ - session_handle_lists_.size();
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
				session_handle_creator_.AsyncConnect(endpoint_, std::bind(&AsyncReconnectHandle::ConnectCallback, std::move(handler), std::placeholders::_1));
			}
		}
	}
}

// 异步重连结果
void RouterClient::AsyncReconnectResult(AsyncReconnectHandle &handler, asio::error_code error_code)
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

// 获取会话处理器
RouterSessionHandle* RouterClient::GetRouterSessionHandle()
{
	if (session_handle_lists_.size() < connection_num_)
	{
		AsyncReconnect();
		if (session_handle_lists_.empty())
		{
			std::cerr << "No more connections available!" << std::endl;
			assert(false);
			return nullptr;
		}
	}

	if (next_client_index_ >= session_handle_lists_.size())
	{
		next_client_index_ = 0;
	}
	return session_handle_lists_[next_client_index_++];
}

// 更新计时器
void RouterClient::OnUpdateTimer(asio::error_code error_code)
{
	if (error_code)
	{
		std::cerr << error_code.message() << std::endl;
		return;
	}

	for (auto session : session_handle_lists_)
	{
		session->HeartbeatCountdown();
	}
	timer_.expires_from_now(std::chrono::seconds(1));
	timer_.async_wait(wait_handler_);
}

// 连接事件
void RouterClient::OnConnected(RouterSessionHandle *session)
{
	if (session_handle_lists_.size() >= connection_num_)
	{
		assert(false);
		session->Close();
		return;
	}

	if (std::find(session_handle_lists_.begin(), session_handle_lists_.end(), session) == session_handle_lists_.end())
	{
		session_handle_lists_.push_back(session);
	}
}

// 断开连接事件
void RouterClient::OnDisconnect(RouterSessionHandle *session)
{
	lifetimes_.erase(session->GetShared());
	auto session_iter = std::find(session_handle_lists_.begin(), session_handle_lists_.end(), session);
	if (session_iter != session_handle_lists_.end())
	{
		session_handle_lists_.erase(session_iter);
	}
}

// 接受消息事件
void RouterClient::OnMessage(RouterSessionHandle *session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	if (dynamic_cast<internal::RouterNotify*>(message) != nullptr)
	{
		auto response = static_cast<internal::RouterNotify*>(message);
		context_.node_type = response->src_type();
		context_.child_id = response->src_child_id();
		assert(response->message_length() == buffer.Readable());
		if (response->message_length() == buffer.Readable())
		{
			auto forward_message = ProtubufCodec::Decode(buffer);
			assert(forward_message != nullptr);
			if (forward_message != nullptr && message_cb_ != nullptr)
			{
				message_cb_(this, forward_message.get(), buffer);
			}
		}
	}
	else if (dynamic_cast<internal::RouterErrorRsp*>(message) != nullptr)
	{
		auto response = static_cast<internal::RouterErrorRsp*>(message);
		std::cerr << "Router error code: " << response->error_code() << ", message name: " << response->what() << std::endl;
	}
	else
	{
		assert(false);
	}
}

// 回复消息
void RouterClient::Reply(google::protobuf::Message *message)
{
	Send(context_.node_type, context_.child_id, message);
}

// 发送消息
void RouterClient::Send(int dst_node_type, int dst_child_id, google::protobuf::Message *message)
{
	assert(dst_node_type >= internal::NodeType_MIN && dst_node_type <= internal::NodeType_MAX);
	RouterSessionHandle *session = GetRouterSessionHandle();
	if (session != nullptr)
	{
		network::NetMessage buffer;
		internal::ForwardReq header;
		header.set_dst_child_id(dst_child_id);
		header.set_dst_type(static_cast<internal::NodeType>(dst_node_type));
		header.set_message_length(message->ByteSize());
		ProtubufCodec::Encode(&header, buffer);
		ProtubufCodec::Encode(message, buffer);
		session->Send(buffer);
	}
}

// 广播消息
void RouterClient::Broadcast(const std::vector<int> &dst_type_lists, google::protobuf::Message *message)
{
	RouterSessionHandle *session = GetRouterSessionHandle();
	if (session != nullptr)
	{
		network::NetMessage buffer;
		internal::BroadcastReq header;
		for (size_t i = 0; i < dst_type_lists.size(); ++i)
		{
			int dst_node_type = dst_type_lists[i];
			assert(dst_node_type >= internal::NodeType_MIN && dst_node_type <= internal::NodeType_MAX);
			header.add_dst_lists(static_cast<internal::NodeType>(dst_node_type));
		}
		header.set_message_length(message->ByteSize());
		ProtubufCodec::Encode(&header, buffer);
		ProtubufCodec::Encode(message, buffer);
		session->Send(buffer);
	}
}
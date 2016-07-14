#include "RouterClient.h"
#include <proto/MessageHelper.h>
#include <proto/internal.protocol.pb.h>

/************************************************************************/

class RouterClientHandle : public network::TCPSessionHandler
{
	friend class RouterClient;

public:
	RouterClientHandle(RouterClient *client, std::shared_ptr<bool> &life)
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
			PackageMessage(&request, message);
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

		auto response = UnpackageMessage(message);
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
			client_->OnMessage(this, response.get());
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
				PackageMessage(&request, message);
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
	, wait_handler_(std::bind(&RouterClient::UpdateTimer, this, std::placeholders::_1))
	, client_creator_(threads_, std::bind(&RouterClient::CreateSessionHandle, this), std::bind(CreaterMessageFilter))
{
	// 节点类型和子节点id检测

	// 检测不成功直接让程序崩溃
}

RouterClient::~RouterClient()
{
	Clear();
}

// 创建会话处理器
network::SessionHandlePointer RouterClient::CreateSessionHandle()
{
	auto life = std::make_shared<bool>();
	lifetimes_.insert(life);
	return std::make_shared<RouterClientHandle>(this, life);
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

// 响应消息
void RouterClient::Respond(google::protobuf::Message *message)
{

}

// 发送消息
void RouterClient::Send(int node_type, int node_id, google::protobuf::Message *message)
{

}
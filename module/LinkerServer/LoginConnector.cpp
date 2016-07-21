#include "LoginConnector.h"
#include <ProtobufCodec.h>
#include <proto/public_struct.pb.h>
#include "Logging.h"

/************************************************************************/

LoginSessionHandle::LoginSessionHandle(LoginConnector *connector, std::shared_ptr<bool> &life)
	: counter_(0)
	, is_logged_(false)
	, connector_life_(life)
	, connector_(connector)
	, heartbeat_interval_(0)
{
}

// 获取计数器
std::shared_ptr<bool>& LoginSessionHandle::GetShared()
{
	return connector_life_;
}

// 连接成功
void LoginSessionHandle::OnConnect()
{
	if (connector_life_.unique())
	{
		Close();
	}
	else
	{
		connector_->OnConnected(this);
	}
}

// 接收消息
void LoginSessionHandle::OnMessage(network::NetMessage &message)
{
	if (connector_life_.unique())
	{
		Close();
	}
	else
	{
		connector_->OnMessage(this, message);
	}
}

// 连接关闭
void LoginSessionHandle::LoginSessionHandle::OnClose()
{
	is_logged_ = false;
	if (!connector_life_.unique())
	{
		connector_->OnDisconnect(this);
	}
}

// 发送心跳倒计时
void LoginSessionHandle::HeartbeatCountdown()
{
	if (is_logged_ && heartbeat_interval_ > 0)
	{
		if (--counter_ == 0)
		{
			pub::PingReq request;
			network::NetMessage message;
			ProtubufCodec::Encode(&request, message);
			Send(message);
			counter_ = heartbeat_interval_;
		}
	}
}

/************************************************************************/
/************************************************************************/

class AsyncReconnectHandle : public std::enable_shared_from_this< AsyncReconnectHandle >
{
public:
	AsyncReconnectHandle(LoginConnector *connector, std::shared_ptr<bool> &life)
		: connector_(connector)
		, connector_life_(life)
	{
	}

	// 获取计数器
	std::shared_ptr<bool>& GetShared()
	{
		return connector_life_;
	}

	// 连接结果回调
	void ConnectCallback(asio::error_code error_code)
	{
		if (!connector_life_.unique())
		{
			connector_->AsyncReconnectResult(*this, error_code);
		}
	}

private:
	LoginConnector*       connector_;
	std::shared_ptr<bool> connector_life_;
};

/************************************************************************/
/************************************************************************/

network::MessageFilterPointer CreaterConnectorMessageFilter()
{
	return std::make_shared<network::DefaultMessageFilter>();
}

LoginConnector::LoginConnector(network::IOServiceThreadManager &threads, asio::ip::tcp::endpoint &endpoint,
	unsigned short patition_id, std::function<void> &callback)
	: threads_(threads)
	, is_logged_(false)
	, session_handle_(nullptr)
	, patition_id_(patition_id)
	, login_callback_(callback)
	, timer_(threads.MainThread()->IOService())
	, wait_handler_(std::bind(&LoginConnector::OnUpdateTimer, this, std::placeholders::_1))
	, session_handle_creator_(threads_, std::bind(&LoginConnector::CreateSessionHandle, this), std::bind(CreaterConnectorMessageFilter))
{
	InitConnections();
	timer_.async_wait(wait_handler_);
}

LoginConnector::~LoginConnector()
{
	Clear();
}


// 清理连接
void LoginConnector::Clear()
{

}

// 创建会话处理器
network::SessionHandlePointer LoginConnector::CreateSessionHandle()
{
	auto life = std::make_shared<bool>();
	lifetimes_.insert(life);
	return std::make_shared<LoginSessionHandle>(this, life);
}

// 初始化连接
void LoginConnector::InitConnections()
{
	asio::error_code error_code;
	try
	{
		session_handle_creator_.Connect(endpoint_, error_code);
	}
	catch (const std::exception&)
	{
		Clear();
		throw ConnectLoginServerFail(error_code.message().c_str());
	}

	if (error_code)
	{
		Clear();
		throw ConnectLoginServerFail(error_code.message().c_str());
	}
}

// 异步重连
void LoginConnector::AsyncReconnect()
{

}

// 异步重连结果
void LoginConnector::AsyncReconnectResult(AsyncReconnectHandle &handler, asio::error_code error_code)
{

}

// 更新计时器
void LoginConnector::OnUpdateTimer(asio::error_code error_code)
{
	if (error_code)
	{
		logger()->error(error_code.message());
		return;
	}

	if (session_handle_ != nullptr)
	{
		session_handle_->HeartbeatCountdown();
	}
	timer_.expires_from_now(std::chrono::seconds(1));
	timer_.async_wait(wait_handler_);
}

// 发送消息
bool LoginConnector::Send(google::protobuf::Message *message)
{
	if (!is_logged_ || session_handle_ == nullptr)
	{
		return false;
	}

	network::NetMessage buffer;
	ProtubufCodec::Encode(message, buffer);
	session_handle_->Send(buffer);
	return true;
}

// 连接事件
void LoginConnector::OnConnected(LoginSessionHandle *session)
{
	assert(!is_logged_ && session_handle_ == nullptr);
	if (!is_logged_ && session_handle_ == nullptr)
	{

	}
}

// 接收消息事件
void LoginConnector::OnMessage(LoginSessionHandle *session, network::NetMessage &buffer)
{

}

// 断开连接事件
void LoginConnector::OnDisconnect(LoginSessionHandle *session)
{

}
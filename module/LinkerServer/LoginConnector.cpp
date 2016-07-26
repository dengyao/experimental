#include "LoginConnector.h"
#include <ProtobufCodec.h>
#include <proto/public_struct.pb.h>
#include <proto/server_internal.pb.h>
#include "Logging.h"
#include "ServerConfig.h"
#include "LoginSessionHandle.h"

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

LoginConnector::LoginConnector(network::IOServiceThreadManager &threads, asio::ip::tcp::endpoint &endpoint, const std::function<void(uint16_t)> &callback)
	: linker_id_(0)
	, threads_(threads)
	, endpoint_(endpoint)
	, reconnecting_(false)
	, session_handle_(nullptr)
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
	lifetimes_.clear();
	if (session_handle_ != nullptr)
	{
		session_handle_->Close();
	}
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
	if (!reconnecting_)
	{
		reconnecting_ = true;
		auto life = std::make_shared<bool>();
		lifetimes_.insert(life);
		auto handler = std::make_shared<AsyncReconnectHandle>(this, life);
		session_handle_creator_.AsyncConnect(endpoint_, std::bind(&AsyncReconnectHandle::ConnectCallback, std::move(handler), std::placeholders::_1));
	}
}

// 异步重连结果
void LoginConnector::AsyncReconnectResult(AsyncReconnectHandle &handler, asio::error_code error_code)
{
	if (error_code)
	{
		reconnecting_ = false;
		logger()->error("重连登录服务器失败，{}", error_code.message());
	}
	lifetimes_.erase(handler.GetShared());
}

// 更新计时器
void LoginConnector::OnUpdateTimer(asio::error_code error_code)
{
	if (error_code)
	{
		logger()->error("{}:{} {}", __FUNCTION__, __LINE__, error_code.message());
		return;
	}

	if (session_handle_ != nullptr)
	{
		session_handle_->HeartbeatCountdown();
	}
	timer_.expires_from_now(std::chrono::seconds(1));
	timer_.async_wait(wait_handler_);
}

// 设置消息回调
void LoginConnector::SetMessageCallback(const Callback &cb)
{
	message_cb_ = cb;
}

// 发送消息
bool LoginConnector::Send(google::protobuf::Message *message)
{
	if (linker_id_ == 0)
	{
		return false;
	}

	if (session_handle_ == nullptr)
	{
		AsyncReconnect();
		return false;
	}
	else
	{
		network::NetMessage buffer;
		ProtubufCodec::Encode(message, buffer);
		session_handle_->Send(buffer);
	}
	return true;
}

// 连接事件
void LoginConnector::OnConnected(LoginSessionHandle *session)
{
	assert(session_handle_ == nullptr);
	if (session_handle_ == nullptr)
	{
		svr::LinkerLoginReq request;
		request.set_port(ServerConfig::GetInstance()->GetPort());
		request.set_public_ip(ServerConfig::GetInstance()->GetPublicIP());
		request.set_partition_id(ServerConfig::GetInstance()->GetPatitionID());
		if (linker_id_ != 0)
		{
			request.set_linker_id(linker_id_);
		}
		network::NetMessage message;
		ProtubufCodec::Encode(&request, message);
		session->Send(message);
	}
}

// 接收消息
void LoginConnector::OnMessage(LoginSessionHandle *session, network::NetMessage &buffer)
{
	auto request = ProtubufCodec::Decode(buffer);
	if (dynamic_cast<svr::LinkerLoginRsp*>(request.get()) != nullptr)
	{
		// 处理登录结果
		assert(session_handle_ == nullptr);
		if (session_handle_ == nullptr)
		{
			svr::LinkerLoginRsp *message = static_cast<svr::LinkerLoginRsp*>(request.get());
			reconnecting_ = false;
			session_handle_ = session;
			linker_id_ = message->linker_id();
			session_handle_->SetHeartbeatInterval(message->heartbeat_interval() / 2);

			// 登录通知只回调一次
			if (login_callback_ != nullptr)
			{
				login_callback_(linker_id_);
				login_callback_ = nullptr;
			}
			else
			{
				logger()->info("重连登录服务器成功!");
			}
		}	
	}
	else if (dynamic_cast<pub::PongRsp*>(request.get()) == nullptr) 
	{
		if (dynamic_cast<pub::ErrorRsp*>(request.get()) != nullptr)
		{
			// 处理错误响应
			pub::ErrorRsp *error = static_cast<pub::ErrorRsp*>(request.get());
			if (error->what() == svr::LinkerLoginReq::default_instance().GetTypeName())
			{
				if (login_callback_ != nullptr)
				{
					login_callback_(0);
					login_callback_ = nullptr;
				}
				else
				{
					logger()->error("重连登录服务器失败，{}", static_cast<int>(error->error_code()));
				}
				return;
			}
		}

		if (session_handle_ != nullptr && message_cb_ != nullptr)
		{
			message_cb_(this, request.get(), buffer);
		}
	}
}

// 断开连接事件
void LoginConnector::OnDisconnect(LoginSessionHandle *session)
{
	if (session_handle_ != nullptr)
	{
		assert(session == session_handle_);
		session_handle_ = nullptr;
	}
	reconnecting_ = false;
	lifetimes_.erase(session->GetShared());
	logger()->error("与登录服务器断开连接!");
}
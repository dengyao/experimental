#ifndef __LOGIN_CONNECTOR_H__
#define __LOGIN_CONNECTOR_H__

#include <set>
#include <network.h>

namespace google
{
	namespace protobuf
	{
		class Message;
	}
}

class LoginConnector;

class ConnectLoginServerFail : public std::runtime_error
{
public:
	ConnectLoginServerFail(const char *message)
		: std::runtime_error(message)
	{
	}
};

class LoginSessionHandle : public network::TCPSessionHandler
{
public:
	LoginSessionHandle(LoginConnector *connector, std::shared_ptr<bool> &life);

public:
	virtual void OnConnect();

	virtual void OnMessage(network::NetMessage &message);

	virtual void OnClose();

public:
	void HeartbeatCountdown();

	std::shared_ptr<bool>& GetShared();

private:
	LoginConnector*       connector_;
	bool                  is_logged_;
	std::shared_ptr<bool> connector_life_;
	uint32_t              counter_;
	uint32_t              heartbeat_interval_;
};

class LoginConnector
{
	friend class LoginSessionHandle;
	friend class AsyncReconnectHandle;

public:
	LoginConnector(network::IOServiceThreadManager &threads, asio::ip::tcp::endpoint &endpoint, std::function<void(uint16_t)> &callback);
	~LoginConnector();

public:
	// 发送消息
	bool Send(google::protobuf::Message *message);
	
private:
	// 连接事件
	void OnConnected(LoginSessionHandle *session);

	// 接收消息事件
	void OnMessage(LoginSessionHandle *session, network::NetMessage &buffer);

	// 断开连接事件
	void OnDisconnect(LoginSessionHandle *session);

private:
	// 清理连接
	void Clear();

	// 创建会话处理器
	network::SessionHandlePointer CreateSessionHandle();

	// 初始化连接
	void InitConnections();

	// 异步重连
	void AsyncReconnect();

	// 异步重连结果
	void AsyncReconnectResult(AsyncReconnectHandle &handler, asio::error_code error_code);

	// 更新计时器
	void OnUpdateTimer(asio::error_code error_code);

private:
	uint16_t                                    linker_id_;
	network::IOServiceThreadManager&            threads_;
	std::function<void(uint16_t)>               login_callback_;
	network::TCPClient					        session_handle_creator_;
	std::set<std::shared_ptr<bool> >            lifetimes_;
	asio::ip::tcp::endpoint                     endpoint_;
	asio::steady_timer                          timer_;
	const std::function<void(asio::error_code)> wait_handler_;
	LoginSessionHandle*                         session_handle_;
};

#endif

#ifndef __LINKER_MANAGER_H__
#define __LINKER_MANAGER_H__

#include <unordered_set>
#include <network.h>

namespace router
{
	class Connector;
}

namespace google
{
	namespace protobuf
	{
		class Message;
	}
}

class SessionHandle;
class LoginConnector;

// 用户鉴权信息
struct SUserAuth
{
	uint32_t user_id;
	std::chrono::steady_clock::time_point time;

	SUserAuth()
		: user_id(0), time(std::chrono::steady_clock::now())
	{
	}
};

class LinkerManager
{
public:
	LinkerManager(network::IOServiceThreadManager &threads);

public:
	// 处理用户连接
	void HandleUserConnected(SessionHandle *session);

	// 处理来自用户的消息
	void HandleMessageFromUser(SessionHandle *session, google::protobuf::Message *messsage, network::NetMessage &buffer);

	// 处理用户关闭连接
	void HandleUserClose(SessionHandle *session);

	// 回复错误码
	void RespondErrorCodeToUser(SessionHandle *session, network::NetMessage &buffer, int error_code, const char *what = nullptr);

	// 处理来自路由的消息
	void HandleMessageFromRouter(router::Connector *connector, google::protobuf::Message *messsage, network::NetMessage &buffer);

	// 处理来自登录服务器的消息
	void HandleMessageFromLoginServer(LoginConnector *connector, google::protobuf::Message *messsage, network::NetMessage &buffer);

private:
	// 更新定时器
	void OnUpdateTimer(asio::error_code error_code);

private:
	asio::steady_timer									timer_;
	const std::function<void(asio::error_code)>			wait_handler_;
	network::IOServiceThreadManager&					threads_;
	std::unordered_map<uint64_t, SUserAuth>				user_auth_;
	std::unordered_map<uint32_t, network::TCPSessionID>	user_session_;
	std::unordered_map<network::TCPSessionID, uint32_t>	reverse_user_session_;
	std::unordered_set<network::TCPSessionID>			unauth_user_session_;
};

#endif
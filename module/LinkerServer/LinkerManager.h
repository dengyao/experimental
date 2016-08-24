#ifndef __LINKER_MANAGER_H__
#define __LINKER_MANAGER_H__

#include <unordered_set>
#include <network.h>

namespace gw
{
	class GWClient;
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

// 用户会话信息
struct SUserSession
{
	uint64_t token;
	network::TCPSessionID session_id;

	SUserSession()
		: token(0), session_id(0)
	{
	}
};

class LinkerManager
{
public:
	LinkerManager(network::IOServiceThreadManager &threads);

public:
	// 用户连接
	void OnUserConnect(SessionHandle *session);

	// 用户消息
	void OnUserMessage(SessionHandle *session, google::protobuf::Message *messsage, network::NetMessage &buffer);

	// 用户关闭连接
	void OnUserClose(SessionHandle *session);

	// 登录服务器消息
	void OnLoginServerMessage(LoginConnector *connector, google::protobuf::Message *messsage, network::NetMessage &buffer);

	// 网关服务器消息
	void OnGatewayServerMessage(gw::GWClient *connector, google::protobuf::Message *messsage, network::NetMessage &buffer);

	// 回复错误码
	void SenddErrorCodeToUser(SessionHandle *session, network::NetMessage &buffer, int error_code, const char *what = nullptr);

private:
	// 广播用户进入
	void OnBroadcastUserEnter(uint32_t user_id);

	// 广播用户离开
	void OnBroadcastUserLeave(uint32_t user_id);

	// 更新定时器
	void OnUpdateTimer(asio::error_code error_code);

private:
	uint32_t											counter_;
	asio::steady_timer									timer_;
	const std::function<void(asio::error_code)>			wait_handler_;
	network::IOServiceThreadManager&					threads_;
	std::unordered_map<uint64_t, SUserAuth>				user_auth_;
	std::unordered_map<uint32_t, SUserSession>	        user_session_;
	std::unordered_map<network::TCPSessionID, uint32_t>	reverse_user_session_;
	std::unordered_set<network::TCPSessionID>			unauth_user_session_;
};

#endif
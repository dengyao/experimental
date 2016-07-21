#ifndef __LOGIN_MANAGER_H__
#define __LOGIN_MANAGER_H__

#include <network.h>
#include "TokenGenerator.h"

namespace google
{
	namespace protobuf
	{
		class Message;
	}
}

class SessionHandle;

struct SPartition
{
	int id;										// 分区id
	std::string name;							// 分区名称
	int status;									// 分区状态
	bool is_recommend;							// 推荐分区
	std::string createtime;						// 创建时间

	SPartition()
		: id(0), status(0), is_recommend(false)
	{
	}
};

struct SLinkerItem
{
	uint16_t linker_id;							// id
	std::string public_ip;						// 公网地址
	unsigned short port;						// 端口
	network::TCPSessionID session_id;			// 会话
	uint32_t online_number;						// 在线人数

	SLinkerItem()
		: linker_id(0), session_id(0), online_number(0)
	{
	}
};

struct SLinkerGroup
{
	uint16_t partition_id;						// 分区id
	std::vector<SLinkerItem> linker_lists;		// linker列表
	network::IDGenerator generator;             // id生成器

	SLinkerGroup()
		: partition_id(0)
		, generator(std::numeric_limits<uint16_t>::max())
	{
	}
};

struct SConnection
{
	uint32_t user_id;							// 用户id
	std::chrono::steady_clock::time_point time;	// 连接时间
	SConnection()
		: user_id(0), time(std::chrono::steady_clock::now())
	{
	}
};

class LoginManager
{
public:
	LoginManager(network::IOServiceThreadManager &threads, const std::vector<SPartition> &partition);

public:
	// 处理接受新连接
	void HandleAcceptConnection(SessionHandle &session);

	// 处理用户消息
	bool HandleUserMessage(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 处理用户离线
	void HandleUserOffline(SessionHandle &session);

	// 处理Linker消息
	bool HandleLinkerMessage(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 处理Linker下线
	void HandleLinkerOffline(SessionHandle &session);

	// 回复错误码
	void RespondErrorCode(SessionHandle &session, network::NetMessage &buffer, int error_code, const char *what = nullptr);

private:
	// 更新定时器
	void OnUpdateTimer(asio::error_code error_code);

private:
	// Linker登录
	bool OnLinkerLogin(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer);

	// Linker上报负载量
	bool OnLinkerUpdateLoadCapacity(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer);

private:
	// 查询分区
	void OnUserQueryPartition(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 用户注册
	void OnUserSignUp(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 用户登录
	void OnUserSignIn(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 进入分区
	void OnUserEntryPartition(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer);

private:
	asio::steady_timer                                     timer_;
	const std::function<void(asio::error_code)>            wait_handler_;
	network::IOServiceThreadManager&                       threads_;
	TokenGenerator                                         generator_;
	std::vector<SPartition>                                partition_lists_;
	std::unordered_map<uint16_t, SLinkerGroup>             partition_map_;
	std::unordered_map<network::TCPSessionID, SConnection> connection_map_;
};

#endif

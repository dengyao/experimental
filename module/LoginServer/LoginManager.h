#ifndef __LOGIN_MANAGER_H__
#define __LOGIN_MANAGER_H__

#include <network.h>
#include "TokenGenerator.h"
#include <ProtobufDispatcher.h>

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

struct SUserSession
{
	uint32_t user_id;							// 用户id
	std::chrono::steady_clock::time_point time;	// 连接时间
	SUserSession()
		: user_id(0), time(std::chrono::steady_clock::now())
	{
	}
};

class LoginManager
{
public:
	LoginManager(network::IOServiceThreadManager &threads, const std::vector<SPartition> &partition);

public:
	// 用户连接
	void OnConnect(SessionHandle *session);

	// 接收用户消息
	bool OnUserMessage(SessionHandle *session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 用户关闭连接
	void OnUserClose(SessionHandle *session);

	// 接收Linker消息
	bool OnLinkerMessage(SessionHandle *session, google::protobuf::Message *message, network::NetMessage &buffer);

	// Linker关闭连接
	void OnLinkerClose(SessionHandle *session);

	// 回复错误码
	void SendErrorCode(SessionHandle *session, network::NetMessage &buffer, int error_code, const char *what = nullptr);

private:
	// 从数据库查询分区信息
	void QueryPartitionInfoByDatabase();

	// 更新定时器
	void OnUpdateTimer(asio::error_code error_code);

private:
	// 未定义消息
	bool OnUnknownMessage(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer);

private:
	// Linker登录
	bool OnLinkerLogin(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer);

	// Linker上报负载量
	bool OnReportLinker(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer);

private:
	// 查询分区
	bool OnUserQueryPartition(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 用户注册
	bool OnUserSignUp(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 用户登录
	bool OnUserSignIn(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 进入分区
	bool OnUserEntryPartition(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer);

private:
	asio::steady_timer										timer_;
	const std::function<void(asio::error_code)>				wait_handler_;
	network::IOServiceThreadManager&						threads_;
	TokenGenerator											generator_;
	std::vector<SPartition>									partition_lists_;
	std::unordered_map<uint16_t, SLinkerGroup>				partition_map_;
	std::unordered_map<network::TCPSessionID, SUserSession>	user_session_;
	ProtobufDispatcher										dispatcher_;
};

#endif

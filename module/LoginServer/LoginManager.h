#ifndef __LOGIN_MANAGER_H__
#define __LOGIN_MANAGER_H__

#include <network.h>

namespace google
{
	namespace protobuf
	{
		class Message;
	}
}

struct SPartition
{
	int id;					// 分区id
	std::string name;		// 分区名称
	int status;				// 分区状态
	bool is_recommend;		// 推荐分区
	std::string createtime;	// 创建时间
	SPartition()
		: id(0), status(0), is_recommend(false)
	{
	}
};

class SessionHandle;

class LoginManager
{
public:
	LoginManager(network::IOServiceThreadManager &threads, const std::vector<SPartition> &partition);

public:
	// 处理用户消息
	bool HandleUserMessage(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 处理Linker消息
	bool HandleLinkerMessage(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 回复错误码
	void RespondErrorCode(SessionHandle &session, network::NetMessage &buffer, int error_code, const char *what = nullptr);

private:
	// Linker注册
	bool OnLinkerRegister(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer);

	// Linker上报负载量
	bool OnLinkerUpdateLoadCapacity(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer);

private:
	// 用户登录
	bool OnUserSignIn(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 用户注册
	bool OnUserSignUp(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 查询分区
	bool OnUserQueryPartition(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 进入分区
	bool OnUserEntryPartition(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer);

private:
	network::IOServiceThreadManager& threads_;
	std::vector<SPartition>          partition_lists_;
};

#endif

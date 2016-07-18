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

class SessionHandle;

class LoginManager
{
private:
	// 用户登录
	void OnUserSignIn(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 用户注册
	void OnUserSignUp(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 查询分区
	void OnUserQueryPartition(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 进入分区
	void OnUserEntryPartition(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer);
};

#endif

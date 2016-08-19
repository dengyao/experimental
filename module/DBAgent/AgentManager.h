﻿#ifndef __AGENT_MANAGER_H__
#define __AGENT_MANAGER_H__

#include <network.h>
#include "AgentMySQL.h"
#include "ConnectorMySQL.h"
#include <ProtobufDispatcher.h>

namespace google
{
	namespace protobuf
	{
		class Message;
	}
}

class SessionHandle;

class AgentManager
{
	// 请求来源信息
	struct SSourceInfo
	{
		ActionType type;
		uint32_t sequence;
		network::TCPSessionID session_id;
		SSourceInfo(ActionType type, uint32_t seq, network::TCPSessionID id)
			: type(type), sequence(seq), session_id(id)
		{
		}
	};

public:
	AgentManager(network::IOServiceThreadManager &threads, AgentMySQL &mysql, unsigned int backlog);

public:
	// 接收消息
	bool OnMessage(SessionHandle *session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 回复错误码
	void SendErrorCode(SessionHandle *session, uint32_t sequence, int error_code, network::NetMessage &buffer);

	// 回复处理结果
	void SendHandleResult(network::TCPSessionID id, uint32_t sequence, const HandleResult &result, network::NetMessage &buffer);

private:
	// 更新处理结果
	void UpdateHandleResult(asio::error_code error_code);

	// 更新统计数据
	void UpdateStatisicalData(asio::error_code error_code);

private:
	// 未定义消息
	bool OnUnknownMessage(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 查询代理信息
	bool OnQueryAgentInfo(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 操作数据库
	bool OnHandleDatabase(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer);

private:
	asio::steady_timer							timer_;
	asio::steady_timer							statisical_timer_;
	const std::function<void(asio::error_code)>	wait_handler_;
	const std::function<void(asio::error_code)>	statisical_wait_handler_;
	network::IOServiceThreadManager&			threads_;
	std::map<uint32_t, SSourceInfo>				requests_;
	network::IDGenerator                        generator_;
	AgentMySQL&									mysql_agent_;
	std::vector<HandleResult>					completion_lists_;
	ProtobufDispatcher							dispatcher_;
};

#endif

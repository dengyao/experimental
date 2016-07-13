#ifndef __AGENT_MANAGER_H__
#define __AGENT_MANAGER_H__

#include <network.h>
#include "AgentImpl.h"
#include "ConnectorMySQL.h"

namespace google
{
	namespace protobuf
	{
		class Message;
	}
}

class AgentManager
{
	// 请求来源信息
	struct SSourceInfo
	{
		uint32_t sequence;
		network::TCPSessionID session_id;
		SSourceInfo(uint32_t seq, network::TCPSessionID id)
			: sequence(seq), session_id(id)
		{
		}
	};

public:
	AgentManager(network::IOServiceThreadManager &threads, AgentImpl<MySQL> &mysql, unsigned int backlog);

public:
	// 接受处理请求
	void HandleMessage(network::TCPSessionHandler &session, google::protobuf::Message *message);

	// 回复错误码
	void RespondErrorCode(network::TCPSessionHandler &session, uint32_t sequence, int error_code);

	// 回复处理结果
	void RespondHandleResult(network::TCPSessionID id, uint32_t sequence, const Result &result);

private:
	// 更新处理结果
	void UpdateHandleResult(asio::error_code error_code);

private:
	asio::steady_timer                          timer_;
	const std::function<void(asio::error_code)> wait_handler_;
	network::IOServiceThreadManager&               threads_;
	std::map<uint32_t, SSourceInfo>             requests_;
	network::IDGenerator                           generator_;
	AgentImpl<MySQL>&                           mysql_proxy_;
	std::vector<Result>                         completion_list_;
};

#endif

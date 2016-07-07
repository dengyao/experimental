#ifndef __PROXY_MANAGER_H__
#define __PROXY_MANAGER_H__

#include <eddy.h>
#include "ProxyImpl.h"
#include "ConnectorMySQL.h"

namespace google
{
	namespace protobuf
	{
		class Message;
	}
}

namespace dbproxy
{
	class ProxyManager
	{
		// 请求来源信息
		struct SSourceInfo
		{
			uint32_t sequence;
			eddy::TCPSessionID session_id;
			SSourceInfo(uint32_t seq, eddy::TCPSessionID id)
				: sequence(seq), session_id(id)
			{
			}
		};

	public:
		ProxyManager(eddy::IOServiceThreadManager &threads, dbproxy::ProxyImpl<dbproxy::MySQL> &mysql);

		// 接受处理请求
		void HandleMessage(eddy::TCPSessionHandler &session, google::protobuf::Message *message);

		// 回复错误码
		void RespondErrorCode(eddy::TCPSessionHandler &session, uint32_t sequence, int error_code);

		// 回复处理结果
		void RespondHandleResult(eddy::TCPSessionID id, uint32_t sequence, const dbproxy::Result &result);

	private:
		// 更新处理结果
		void UpdateHandleResult(asio::error_code error_code);

	private:
		asio::steady_timer                  timer_;
		eddy::IOServiceThreadManager&       threads_;
		std::map<uint32_t, SSourceInfo>     requests_;
		eddy::IDGenerator                   generator_;
		dbproxy::ProxyImpl<dbproxy::MySQL>& mysql_proxy_;
		std::vector<dbproxy::Result>        completion_list_;
	};
}

#endif

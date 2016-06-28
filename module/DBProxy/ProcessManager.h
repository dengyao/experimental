#ifndef __PROCESS_MANAGER_H__
#define __PROCESS_MANAGER_H__

#include "eddy.h"
#include "ProxyManager.h"
#include "ConnectorMySQL.h"

class ProcessManager
{
public:
	ProcessManager(eddy::IOServiceThreadManager &threads, dbproxy::ProxyManager<dbproxy::MySQL> &mysql);

public:
	void HandleMessage(eddy::TCPSessionHandle &session, eddy::NetMessage &message);

private:
	void UpdateHandleResult(asio::error_code error_code);

	void ReplyErrorCode(eddy::TCPSessionHandle &session, int error_code);

	void ReplyErrorCode(eddy::TCPSessionHandle &session, uint32_t identifier, int error_code);

	void ReplyHandleResult(eddy::TCPSessionID id, uint32_t identifier, const dbproxy::Result &result);

private:
	struct SourceInfo
	{
		uint32_t identifier;
		eddy::TCPSessionID session_id;
		SourceInfo(uint32_t args1, eddy::TCPSessionID args2)
			: identifier(args1)
			, session_id(args2)
		{
		}
	};

	asio::steady_timer                     timer_;
	eddy::IOServiceThreadManager&          threads_;
	std::map<uint32_t, SourceInfo>         requests_;
	eddy::IDGenerator                      generator_;
	dbproxy::ProxyManager<dbproxy::MySQL>& mysql_proxy_;
	std::vector<dbproxy::Result>           completion_list_;
};

#endif

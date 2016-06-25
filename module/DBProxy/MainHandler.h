#ifndef __MAIN_PROCESSOR_H__
#define __MAIN_PROCESSOR_H__

#include "eddy.h"
#include "ProxyManager.h"
#include "ConnectorMySQL.h"

class MainHandler
{
public:
	MainHandler(eddy::IOServiceThreadManager &threads, dbproxy::ProxyManager<dbproxy::MySQL> &mysql);

public:
	void HandleMessage(eddy::TCPSessionHandle &session, eddy::NetMessage &message);

private:
	void UpdateHandleResult(asio::error_code error_code);

	void ReplyHandleResult(eddy::TCPSessionID id, eddy::NetMessage &message);

private:
	struct SourceInfo
	{
		int number;
		eddy::TCPSessionID session_id;
		SourceInfo(int args1, eddy::TCPSessionID args2)
			: number(args1)
			, session_id(args2)
		{
		}
	};

private:
	asio::steady_timer                     loop_;
	eddy::IOServiceThreadManager&          threads_;
	std::map<uint32_t, SourceInfo>         requests_;
	eddy::IDGenerator                      generator_;
	dbproxy::ProxyManager<dbproxy::MySQL>& mysql_proxy_;
	std::vector<dbproxy::Result>           completion_list_;
};

#endif

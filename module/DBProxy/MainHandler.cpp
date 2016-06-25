#include "MainHandler.h"
#include <iostream>
#include "proto/db.request.pb.h"
#include "proto/db.response.pb.h"


MainHandler::MainHandler(eddy::IOServiceThreadManager &threads, dbproxy::ProxyManager<dbproxy::MySQL> &mysql)
	: threads_(threads)
	, generator_(65535)
	, mysql_proxy_(mysql)
	, loop_(threads.MainThread()->IOService())
{
	loop_.async_wait(std::bind(&MainHandler::UpdateHandleResult, this, std::placeholders::_1));
}

void MainHandler::HandleMessage(eddy::TCPSessionHandle &session, eddy::NetMessage &message)
{
	proto_db::Request request;
	if (request.ParseFromArray(message.Data(), message.Readable()))
	{
		uint32_t number = 0;
		if (generator_.Get(number))
		{
			requests_.insert(std::make_pair(number, SourceInfo(request.number(), session.SessionID())));
			if (!mysql_proxy_.Append(number, (dbproxy::ActionType)request.action(), request.dbname().c_str(), request.statement().c_str(), request.statement().size()))
			{
				return;
			}
		}
		else
		{
			return;
		}
	}
	else
	{
		return;
	}
}

void MainHandler::UpdateHandleResult(asio::error_code error_code)
{
	if (error_code)
	{
		std::cerr << error_code.message() << std::endl;
		return;
	}

	assert(completion_list_.empty());
	mysql_proxy_.GetCompletionQueue(completion_list_);

	for (const auto &item : completion_list_)
	{

	}
	completion_list_.clear();

	loop_.async_wait(std::bind(&MainHandler::UpdateHandleResult, this, std::placeholders::_1));
}

void MainHandler::ReplyHandleResult(eddy::TCPSessionID id, eddy::NetMessage &message)
{

}
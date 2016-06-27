#include "MainHandler.h"
#include <iostream>
#include "proto/db.request.pb.h"
#include "proto/db.response.pb.h"


inline dbproxy::ActionType ToLocalActionType(proto_db::Request_ActoinType type)
{
	switch (type)
	{
	case proto_db::Request_ActoinType_kSelect:
		return dbproxy::ActionType::kSelect;
	case proto_db::Request_ActoinType_kInsert:
		return dbproxy::ActionType::kInsert;
	case proto_db::Request_ActoinType_kUpdate:
		return dbproxy::ActionType::kUpdate;
	case proto_db::Request_ActoinType_kDelete:
		return dbproxy::ActionType::kDelete;
	default:
		throw dbproxy::NotFoundDatabase();
	}
}

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
		uint32_t local_identifier = 0;
		if (generator_.Get(local_identifier))
		{
			try
			{
				requests_.insert(std::make_pair(local_identifier, SourceInfo(request.identifier(), session.SessionID())));
				mysql_proxy_.Append(local_identifier, ToLocalActionType(request.action()), request.dbname().c_str(), request.statement().c_str(), request.statement().size());
			}
			catch (dbproxy::NotFoundDatabase &e)
			{
				requests_.erase(local_identifier);

			}
			catch (dbproxy::ResourceInsufficiency &e)
			{
				requests_.erase(local_identifier);
			}
		}
		else
		{

		}
	}
	else
	{

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

	for (const auto &result : completion_list_)
	{
		auto found = requests_.find(result.GetIdentifier());
		assert(found != requests_.end());
		if (found != requests_.end())
		{
			ReplyHandleResult(found->second.session_id, found->second.identifier, result);
		}
	}
	completion_list_.clear();

	loop_.async_wait(std::bind(&MainHandler::UpdateHandleResult, this, std::placeholders::_1));
}

void MainHandler::ReplyHandleResult(eddy::TCPSessionID id, uint32_t identifier, const dbproxy::Result &result)
{
	eddy::SessionHandlerPointer session = threads_.SessionHandler(id);
	if (session != nullptr)
	{
		eddy::NetMessage message;
		if (!result.GetErrorCode())
		{
			proto_db::Response response;
			response.set_identifier(identifier);
			response.set_result(result.GetResult().data(), result.GetResult().size());

		}
		else
		{
			proto_db::DBError error;
			error.set_identifier(identifier);
			error.set_error_code(result.GetErrorCode().Value());
			error.set_what(result.GetErrorCode().Message());

		}
		session->Send(message);
	}
}
#include "ProcessManager.h"
#include <iostream>
#include "proto/db.request.pb.h"
#include "proto/db.response.pb.h"

inline bool ToLocalActionType(proto_db::Request::ActoinType type, dbproxy::ActionType &local_type)
{
	switch (type)
	{
	case proto_db::Request::kSelect:
		local_type = dbproxy::ActionType::kSelect;
	case proto_db::Request::kInsert:
		local_type = dbproxy::ActionType::kInsert;
	case proto_db::Request::kUpdate:
		local_type = dbproxy::ActionType::kUpdate;
	case proto_db::Request::kDelete:
		local_type = dbproxy::ActionType::kDelete;
	default:
		return false;
	}
	return true;
}

ProcessManager::ProcessManager(eddy::IOServiceThreadManager &threads, dbproxy::ProxyManager<dbproxy::MySQL> &mysql)
	: threads_(threads)
	, generator_(65535)
	, mysql_proxy_(mysql)
	, timer_(threads.MainThread()->IOService())
{
	timer_.async_wait(std::bind(&ProcessManager::UpdateHandleResult, this, std::placeholders::_1));
}

void ProcessManager::HandleMessage(eddy::TCPSessionHandle &session, eddy::NetMessage &message)
{
	proto_db::Request request;
	if (request.ParseFromArray(message.Data(), message.Readable()))
	{
		uint32_t local_identifier = 0;
		if (generator_.Get(local_identifier))
		{
			dbproxy::ActionType local_type;
			if (ToLocalActionType(request.action(), local_type))
			{
				try
				{
					proto_db::Request::DatabaseType dbtype = request.dbtype();
					requests_.insert(std::make_pair(local_identifier, SourceInfo(request.identifier(), session.SessionID())));
					if (dbtype == proto_db::Request::kMySQL)
					{
						mysql_proxy_.Append(local_identifier, local_type, request.dbname().c_str(), request.statement().c_str(), request.statement().size());
					}
					else
					{
						requests_.erase(local_identifier);
						assert(false);
					}
				}
				catch (dbproxy::NotFoundDatabase&)
				{
					requests_.erase(local_identifier);
					ReplyErrorCode(session, request.identifier(), proto_db::ProxyError::kNotFoundDatabase);
				}
				catch (dbproxy::ResourceInsufficiency&)
				{
					requests_.erase(local_identifier);
					ReplyErrorCode(session, request.identifier(), proto_db::ProxyError::kResourceInsufficiency);
				}
			}
			else
			{
				ReplyErrorCode(session, request.identifier(), proto_db::ProxyError::kInvalidAction);
			}
		}
		else
		{
			ReplyErrorCode(session, request.identifier(), proto_db::ProxyError::kResourceInsufficiency);
		}
	}
	else
	{
		ReplyErrorCode(session, 0, proto_db::ProxyError::kInvalidProtocol);
	}
}

void ProcessManager::UpdateHandleResult(asio::error_code error_code)
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
		requests_.erase(found);
	}
	completion_list_.clear();

	timer_.async_wait(std::bind(&ProcessManager::UpdateHandleResult, this, std::placeholders::_1));
}

void ProcessManager::ReplyErrorCode(eddy::TCPSessionHandle &session, uint32_t identifier, int error_code)
{
	eddy::NetMessage message;
	proto_db::ProxyError error;
	error.set_identifier(identifier);
	error.set_error_code(static_cast<proto_db::ProxyError::ErrorCode>(error_code));
	int size = error.ByteSize();
	message.EnsureWritableBytes(size);
	error.SerializeToArray(message.Data(), size);
	message.HasWritten(size);
}

void ProcessManager::ReplyHandleResult(eddy::TCPSessionID id, uint32_t identifier, const dbproxy::Result &result)
{
	eddy::SessionHandlerPointer session = threads_.SessionHandler(id);
	if (session != nullptr)
	{
		eddy::NetMessage message;
		if (result.GetErrorCode())
		{
			proto_db::DBError error;
			error.set_identifier(identifier);
			error.set_error_code(result.GetErrorCode().Value());
			error.set_what(result.GetErrorCode().Message());
			int size = error.ByteSize();
			message.EnsureWritableBytes(size);
			error.SerializeToArray(message.Data(), size);
			message.HasWritten(size);
		}
		else
		{
			proto_db::Response response;
			response.set_identifier(identifier);
			response.set_result(result.GetResult().data(), result.GetResult().size());
			int size = response.ByteSize();
			message.EnsureWritableBytes(size);
			response.SerializeToArray(message.Data(), size);
			message.HasWritten(size);
		}
		session->Send(message);
	}
}
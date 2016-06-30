#include "ProcessManager.h"
#include <limits>
#include <iostream>
#include <proto/dbproxy/dbproxy.Request.pb.h>
#include <proto/dbproxy/dbproxy.Response.pb.h>

typedef std::shared_ptr<google::protobuf::Message> MessagePointer;

void PackageMessage(google::protobuf::Message *in_message, eddy::NetMessage &out_message)
{
	const std::string &full_name = in_message->GetDescriptor()->full_name();
	const uint8_t name_size = static_cast<uint8_t>(full_name.size()) + 1;
	assert(name_size < std::numeric_limits<uint8_t>::max());
	if (name_size >= std::numeric_limits<uint8_t>::max())
	{
		return;
	}

	const size_t byte_size = in_message->ByteSize();
	out_message.EnsureWritableBytes(sizeof(uint8_t) + name_size + byte_size);
	out_message.WriteUInt8(name_size);
	out_message.Write(full_name.data(), name_size);
	in_message->SerializePartialToArray(out_message.Data(), byte_size);
	out_message.HasWritten(byte_size);
}

MessagePointer UnpackageMessage(eddy::NetMessage &in_message)
{
	if (in_message.Readable() == 0)
	{
		return nullptr;
	}

	const uint8_t name_size = in_message.ReadUInt8();
	assert(name_size <= in_message.Readable());
	if (name_size > in_message.Readable())
	{
		return nullptr;
	}

	const std::string full_name(reinterpret_cast<const char*>(in_message.Data()), name_size);
	in_message.Retrieve(name_size);

	auto descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(full_name);
	assert(descriptor != nullptr);
	if (descriptor == nullptr)
	{
		return nullptr;
	}

	auto message_factory = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
	assert(message_factory != nullptr);
	if (message_factory == nullptr)
	{
		return nullptr;
	}

	auto message = MessagePointer(message_factory->New());
	bool parse_success = message->ParseFromArray(in_message.Data(), in_message.Readable());
	in_message.Retrieve(in_message.Readable());
	if (!parse_success)
	{
		return nullptr;
	}

	return message;
}

inline bool ToLocalActionType(proto_dbproxy::Request::ActoinType type, dbproxy::ActionType &local_type)
{
	switch (type)
	{
	case proto_dbproxy::Request::kSelect:
		local_type = dbproxy::ActionType::kSelect;
		break;
	case proto_dbproxy::Request::kInsert:
		local_type = dbproxy::ActionType::kInsert;
		break;
	case proto_dbproxy::Request::kUpdate:
		local_type = dbproxy::ActionType::kUpdate;
		break;
	case proto_dbproxy::Request::kDelete:
		local_type = dbproxy::ActionType::kDelete;
		break;
	default:
		return false;
	}
	return true;
}

ProcessManager::ProcessManager(eddy::IOServiceThreadManager &threads, dbproxy::ProxyManager<dbproxy::MySQL> &mysql)
	: threads_(threads)
	, mysql_proxy_(mysql)
	, timer_(threads.MainThread()->IOService())
	, generator_(std::numeric_limits<uint16_t>::max())
{
	timer_.async_wait(std::bind(&ProcessManager::UpdateHandleResult, this, std::placeholders::_1));
}

void ProcessManager::HandleMessage(eddy::TCPSessionHandler &session, eddy::NetMessage &message)
{
	auto request = std::dynamic_pointer_cast<proto_dbproxy::Request>(UnpackageMessage(message));
	if (request != nullptr)
	{
		uint32_t local_identifier = 0;
		if (generator_.Get(local_identifier))
		{
			dbproxy::ActionType local_type;
			if (ToLocalActionType(request->action(), local_type))
			{
				try
				{
					proto_dbproxy::Request::DatabaseType dbtype = request->dbtype();
					requests_.insert(std::make_pair(local_identifier, SourceInfo(request->identifier(), session.SessionID())));
					if (dbtype == proto_dbproxy::Request::kMySQL)
					{
						mysql_proxy_.Append(local_identifier, local_type, request->dbname().c_str(), request->statement().c_str(), request->statement().size());
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
					ReplyErrorCode(session, request->identifier(), proto_dbproxy::ProxyError::kNotFoundDatabase);
				}
				catch (dbproxy::ResourceInsufficiency&)
				{
					requests_.erase(local_identifier);
					ReplyErrorCode(session, request->identifier(), proto_dbproxy::ProxyError::kResourceInsufficiency);
				}
			}
			else
			{
				ReplyErrorCode(session, request->identifier(), proto_dbproxy::ProxyError::kInvalidAction);
			}
		}
		else
		{
			ReplyErrorCode(session, request->identifier(), proto_dbproxy::ProxyError::kResourceInsufficiency);
		}
	}
	else
	{
		ReplyErrorCode(session, 0, proto_dbproxy::ProxyError::kInvalidProtocol);
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

void ProcessManager::ReplyErrorCode(eddy::TCPSessionHandler &session, uint32_t identifier, int error_code)
{
	proto_dbproxy::ProxyError error;
	error.set_identifier(identifier);
	error.set_error_code(static_cast<proto_dbproxy::ProxyError::ErrorCode>(error_code));

	eddy::NetMessage message;
	PackageMessage(&error, message);
	session.Send(message);
}

void ProcessManager::ReplyHandleResult(eddy::TCPSessionID id, uint32_t identifier, const dbproxy::Result &result)
{
	eddy::SessionHandlePointer session = threads_.SessionHandler(id);
	if (session != nullptr)
	{
		eddy::NetMessage message;
		if (result.GetErrorCode())
		{
			proto_dbproxy::DBError error;
			error.set_identifier(identifier);
			error.set_error_code(result.GetErrorCode().Value());
			error.set_what(result.GetErrorCode().Message());
			PackageMessage(&error, message);
		}
		else
		{
			proto_dbproxy::Response response;
			response.set_identifier(identifier);
			response.set_result(result.GetResult().data(), result.GetResult().size());
			PackageMessage(&response, message);
		}
		session->Send(message);
	}
}
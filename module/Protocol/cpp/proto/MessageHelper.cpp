#include "MessageHelper.h"
#include <eddy/NetMessage.h>
#include <google/protobuf/message.h>
#include "InitProtoDescriptor.h"

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
	in_message->SerializePartialToArray(out_message.Data() + out_message.Readable(), byte_size);
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

	auto message_creator = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
	assert(message_creator != nullptr);
	if (message_creator == nullptr)
	{
		return nullptr;
	}

	auto message = MessagePointer(message_creator->New());
	bool parse_success = message->ParseFromArray(in_message.Data(), in_message.Readable());
	in_message.Retrieve(in_message.Readable());
	if (!parse_success)
	{
		return nullptr;
	}

	return message;
}
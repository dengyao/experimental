#ifndef __MESSAGE_HELPER_H__
#define __MESSAGE_HELPER_H__

#include <memory>

namespace network
{
	class NetMessage;
}

namespace google
{
	namespace protobuf
	{
		class Message;
	}
}

typedef std::unique_ptr<google::protobuf::Message> MessagePointer;

void PackageMessage(google::protobuf::Message *in_message, network::NetMessage &out_message);

MessagePointer UnpackageMessage(network::NetMessage &in_message);

#endif
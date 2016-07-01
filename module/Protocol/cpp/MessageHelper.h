#ifndef __MESSAGE_HELPER_H__
#define __MESSAGE_HELPER_H__

#include <memory>

namespace eddy
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

typedef std::shared_ptr<google::protobuf::Message> MessagePointer;

void PackageMessage(google::protobuf::Message *in_message, eddy::NetMessage &out_message);

MessagePointer UnpackageMessage(eddy::NetMessage &in_message);

#endif
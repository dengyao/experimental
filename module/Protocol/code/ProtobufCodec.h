#ifndef __PROTOBUF_CODEC_H__
#define __PROTOBUF_CODEC_H__

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

class ProtubufCodec
{
public:
	static void Encode(google::protobuf::Message *in_message, network::NetMessage &out_message);

	static MessagePointer Decode(network::NetMessage &in_message);
};

#endif
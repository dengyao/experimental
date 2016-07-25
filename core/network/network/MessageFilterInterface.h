#ifndef __MESSAGE_FILTER_H__
#define __MESSAGE_FILTER_H__

#include <limits>
#include <numeric>
#include "NetMessage.h"

namespace network
{
	class MessageFilterInterface
	{
	public:
		typedef std::vector<uint8_t> Buffer;
		static const size_t kAnyBytes = std::numeric_limits<size_t>::max();

	public:
		MessageFilterInterface() = default;
		virtual ~MessageFilterInterface() = default;

	public:
		virtual size_t BytesWannaRead() = 0;

		virtual size_t BytesWannaWrite(const std::vector<NetMessage> &messages_to_be_sent) = 0;

		virtual size_t Read(const Buffer &buffer, std::vector<NetMessage> &messages_received) = 0;

		virtual size_t Write(const std::vector<NetMessage> &messages_to_be_sent, Buffer &buffer) = 0;

	protected:
		MessageFilterInterface(const MessageFilterInterface&) = delete;
		MessageFilterInterface& operator= (const MessageFilterInterface&) = delete;
	};
}

#endif
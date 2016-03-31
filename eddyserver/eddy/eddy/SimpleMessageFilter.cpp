#include "SimpleMessageFilter.h"
#include <asio/ip/address_v4.hpp>

namespace eddy
{
	const size_t SimpleMessageFilter::s_header_size_ = sizeof(SimpleMessageFilter::MessageHeader);

	SimpleMessageFilter::SimpleMessageFilter()
		: header_read_(false)
	{
	}

	size_t SimpleMessageFilter::BytesWannaRead()
	{
		return header_read_ ? header_ : s_header_size_;
	}

	size_t SimpleMessageFilter::BytesWannaWrite(const std::vector<NetMessage> &messages_to_be_sent)
	{
		if (messages_to_be_sent.empty())
		{
			return 0;
		}
		return std::accumulate(messages_to_be_sent.begin(), messages_to_be_sent.end(), 0, [=](size_t sum, const NetMessage &message)
		{
			return sum + s_header_size_ + message.Readable();
		});
	}

	size_t SimpleMessageFilter::Read(const Buffer &buffer, std::vector<NetMessage> &messages_received)
	{
		if (!header_read_)
		{
			char *data = const_cast<char *>(&*buffer.begin());
			header_ = ntohs(*reinterpret_cast<MessageHeader *>(data));
			header_read_ = true;
			return s_header_size_;
		}
		else
		{
			NetMessage new_message(header_);
			new_message.Write(&*buffer.begin(), header_);
			messages_received.push_back(std::move(new_message));
			header_read_ = false;
			return header_;
		}
	}

	size_t SimpleMessageFilter::Write(const std::vector<NetMessage> &messages_to_be_sent, Buffer &buffer)
	{
		size_t ret = 0;
		for (const NetMessage &message : messages_to_be_sent)
		{
			MessageHeader header = htons(static_cast<MessageHeader>(message.Readable()));
			buffer.insert(buffer.end(), reinterpret_cast<const char*>(&header), reinterpret_cast<const char*>(&header) + sizeof(MessageHeader));
			buffer.insert(buffer.end(), message.Data(), message.Data() + message.Readable());
			ret += s_header_size_ + message.Readable();
		}
		return ret;
	}
}
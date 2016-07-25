#include "DefaultMessageFilter.h"
#include <asio/ip/address_v4.hpp>

namespace network
{
	const size_t DefaultMessageFilter::s_header_size_ = sizeof(DefaultMessageFilter::MessageHeader);

	DefaultMessageFilter::DefaultMessageFilter()
		: header_read_(false)
	{
	}

	size_t DefaultMessageFilter::BytesWannaRead()
	{
		return header_read_ ? header_ : s_header_size_;
	}

	size_t DefaultMessageFilter::BytesWannaWrite(const std::vector<NetMessage> &messages_to_be_sent)
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

	size_t DefaultMessageFilter::Read(const Buffer &buffer, std::vector<NetMessage> &messages_received)
	{
		if (!header_read_)
		{
			uint8_t *data = const_cast<uint8_t *>(buffer.data());
			header_ = ntohs(*reinterpret_cast<MessageHeader *>(data));
			header_read_ = true;
			return s_header_size_;
		}
		else
		{
			NetMessage new_message(header_);
			new_message.Write(buffer.data(), header_);
			messages_received.push_back(std::move(new_message));
			header_read_ = false;
			return header_;
		}
	}

	size_t DefaultMessageFilter::Write(const std::vector<NetMessage> &messages_to_be_sent, Buffer &buffer)
	{
		size_t bytes = 0;
		for (const NetMessage &message : messages_to_be_sent)
		{
			MessageHeader header = htons(static_cast<MessageHeader>(message.Readable()));
			buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&header), reinterpret_cast<const uint8_t*>(&header) + sizeof(MessageHeader));
			buffer.insert(buffer.end(), message.Data(), message.Data() + message.Readable());
			bytes += s_header_size_ + message.Readable();
		}
		return bytes;
	}
}
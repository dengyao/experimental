#include "message_filter.h"
#include <asio/ip/address_v4.hpp>


const size_t SimpleMessageFilter::s_header_size_ = sizeof(SimpleMessageFilter::MessageHeader);

SimpleMessageFilter::SimpleMessageFilter()
	: header_read_(false)
{

}

size_t SimpleMessageFilter::bytes_wanna_read()
{
	return header_read_ ? header_ : s_header_size_;
}

size_t SimpleMessageFilter::bytes_wanna_write(std::vector<NetMessage> &messages_to_be_sent)
{
	if (messages_to_be_sent.empty())
	{
		return 0;
	}
	return std::accumulate(messages_to_be_sent.begin(), messages_to_be_sent.end(), 0, [=](size_t sum, const NetMessage &message)
	{
		return sum + s_header_size_ + message.readable_bytes();
	});
}

size_t SimpleMessageFilter::read(NetMessage &buffer, std::vector<NetMessage> &messages_received)
{
	if (!header_read_)
	{
		header_ = ntohs(buffer.read_uint16());
		header_read_ = true;
		return s_header_size_;
	}
	else
	{
		NetMessage new_message(header_);
		new_message.append(buffer.peek(), header_);
		buffer.retrieve(header_);
		messages_received.push_back(std::move(new_message));
		return header_;
	}
}

size_t SimpleMessageFilter::write(std::vector<NetMessage> &messages_to_be_sent, NetMessage &buffer)
{
	size_t result = 0;
	for (NetMessage &message : messages_to_be_sent)
	{
		MessageHeader lenght = static_cast<MessageHeader>(message.readable_bytes());
		lenght = htons(lenght);
		buffer.append(&lenght, s_header_size_);
		buffer.append(message.peek(), lenght);
		result += s_header_size_ + message.readable_bytes();
	}
	messages_to_be_sent.clear();
	return result;
}
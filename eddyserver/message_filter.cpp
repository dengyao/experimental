#include "message_filter.h"
#include <asio/ip/address_v4.hpp>


const size_t SimpleMessageFilter::s_header_size_ = sizeof(SimpleMessageFilter::MessageHeader);

SimpleMessageFilter::SimpleMessageFilter()
	: header_read_(false)
{

}

size_t SimpleMessageFilter::read(std::vector<NetMessage> &messages_received, NetMessage &message)
{
	if (!header_read_)
	{
		header_ = ntohs(message.read_uint16());
		header_read_ = true;
		return s_header_size_;
	}
	else
	{
		NetMessage new_message(header_);
		new_message.append(message.peek(), header_);
		message.retrieve(header_);
		messages_received.push_back(std::move(new_message));
		return header_;
	}
}

size_t SimpleMessageFilter::write(std::vector<NetMessage> &messages_to_be_sent, NetMessage &message)
{
	return 0;
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
		return sum + s_header_size_ + buffer.size();
	});
}
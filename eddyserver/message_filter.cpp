#include "message_filter.h"
#include <asio/ip/address_v4.hpp>


const size_t SimpleMessageFilter::s_header_size_ = sizeof(SimpleMessageFilter::MessageHeader);

SimpleMessageFilter::SimpleMessageFilter()
	: header_read_(false)
{

}

size_t SimpleMessageFilter::read(std::vector<NETMessage> &messages_received, NETMessage &buffer)
{
	if (!header_read_)
	{
		header_ = ntohs(*reinterpret_cast<MessageHeader *>(buffer.data()));
		header_read_ = true;
		return header_;
	}
	else
	{
		NETMessage message(header_);
		messages_received.push_back();
		return header_;
	}
}

size_t SimpleMessageFilter::write(std::vector<NETMessage> &messages_to_be_sent, NETMessage &buffer)
{
	return 0;
}

size_t SimpleMessageFilter::bytes_wanna_read()
{
	return header_read_ ? header_ : s_header_size_;
}

size_t SimpleMessageFilter::bytes_wanna_write(std::vector<NETMessage> &messages_to_be_sent)
{
	if (messages_to_be_sent.empty())
	{
		return 0;
	}
	return std::accumulate(messages_to_be_sent.begin(), messages_to_be_sent.end(), 0, [=](size_t sum, const NETMessage &buffer)
	{
		return sum + s_header_size_ + buffer.size();
	});
}
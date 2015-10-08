#pragma once

#include <numeric>
#include "types.h"
#include "net_message.h"

class MessageFilterInterface
{
public:
	MessageFilterInterface() = default;
	virtual ~MessageFilterInterface() = default;

public:
	virtual size_t on_read(std::vector<NETMessage> &messages_received, NETMessage &buffer) = 0;
	virtual size_t on_write(std::vector<NETMessage> &messages_to_be_sent, NETMessage &buffer) = 0;
	virtual size_t bytes_wanna_read() = 0;
	virtual size_t bytes_wanna_write(std::vector<NETMessage> &messages_to_be_sent) = 0;
};


class SimpleMessageFilter final : public MessageFilterInterface
{
public:
	typedef uint16_t message_header;
	static const size_t kHeaderSize = sizeof(message_header);

public:
	SimpleMessageFilter();

	~SimpleMessageFilter() = default;

public:
	size_t on_read(message_vector &messages_received, message_buffer &buffer) override;

	size_t on_write(message_vector &messages_to_be_sent, message_buffer &buffer) override;

	size_t bytes_wanna_read() override;

	size_t bytes_wanna_write(message_vector &messages_to_be_sent) override;

private:
	bool			header_read_;
	message_header	header_;
};
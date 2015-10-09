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
	virtual size_t read(std::vector<NETMessage> &messages_received, NETMessage &messages) = 0;
	virtual size_t write(std::vector<NETMessage> &messages_to_be_sent, NETMessage &messages) = 0;
	virtual size_t bytes_wanna_read() = 0;
	virtual size_t bytes_wanna_write(std::vector<NETMessage> &messages_to_be_sent) = 0;
};

class SimpleMessageFilter final : public MessageFilterInterface
{
public:
	typedef uint16_t MessageHeader;

public:
	SimpleMessageFilter();

	~SimpleMessageFilter() = default;

public:
	size_t read(std::vector<NETMessage> &messages_received, NETMessage &buffer) override;

	size_t write(std::vector<NETMessage> &messages_to_be_sent, NETMessage &buffer) override;

	size_t bytes_wanna_read() override;

	size_t bytes_wanna_write(std::vector<NETMessage> &messages_to_be_sent) override;

private:
	bool				header_read_;
	MessageHeader		header_;
	static const size_t	s_header_size_;
};
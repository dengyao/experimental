#pragma once

#include <limits>
#include <numeric>
#include "types.h"
#include "net_message.h"


class MessageFilterInterface
{
public:
	MessageFilterInterface() = default;
	virtual ~MessageFilterInterface() = default;

public:
	size_t any_bytes() const
	{
		return std::numeric_limits<size_t>::max();
	}

public:
	virtual size_t bytes_wanna_read() = 0;
	virtual size_t bytes_wanna_write(std::vector<NetMessage> &messages_to_be_sent) = 0;
	virtual size_t read(NetMessage &buffer, std::vector<NetMessage> &messages_received) = 0;
	virtual size_t write(std::vector<NetMessage> &messages_to_be_sent, NetMessage &buffer) = 0;

protected:
	MessageFilterInterface(const MessageFilterInterface&) = delete;
	MessageFilterInterface& operator= (const MessageFilterInterface&) = delete;
};

class SimpleMessageFilter final : public MessageFilterInterface
{
public:
	typedef uint16_t MessageHeader;

public:
	SimpleMessageFilter();

	~SimpleMessageFilter() = default;

public:
	size_t bytes_wanna_read() override;

	size_t bytes_wanna_write(std::vector<NetMessage> &messages_to_be_sent) override;

	size_t read(NetMessage &buffer, std::vector<NetMessage> &messages_received) override;

	size_t write(std::vector<NetMessage> &messages_to_be_sent, NetMessage &buffer) override;

private:
	bool				header_read_;
	MessageHeader		header_;
	static const size_t	s_header_size_;
};
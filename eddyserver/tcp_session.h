#pragma once

#include <memory>
#include <asio/ip/tcp.hpp>
#include "types.h"

class message_filter;
class io_service_thread;
class message_filter_interface;

static const uint32_t kInvalidSessionID = 0;

class TCPSession final : public std::enable_shared_from_this < TCPSession >
{
	friend class io_service_thread_manager;

public:
	typedef asio::ip::tcp::socket						SocketType;
	typedef std::shared_ptr<message_filter_interface>	message_filter_ptr;

public:
	TCPSession(io_service_thread &thread, message_filter_ptr filter);
	~TCPSession();

public:
	TCPSessionID id() const
	{
		return id_;
	}

	SocketType& socket()
	{
		return socket_;
	}

	io_service_thread& thread() const
	{
		return thread_;
	}

private:
	void init(TCPSessionID id);

private:
	TCPSessionID			id_;
	SocketType				socket_;
	io_service_thread&		thread_;
	message_filter_ptr		filter_;
};
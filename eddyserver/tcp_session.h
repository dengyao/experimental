#pragma once

#include <memory>
#include <asio/ip/tcp.hpp>
#include "types.h"

class IOServiceThread;
class MessageFilterInterface;

class TCPSession final : public std::enable_shared_from_this < TCPSession >
{
	friend class IOServiceThreadManager;

public:
	typedef asio::ip::tcp::socket					SocketType;
	typedef std::shared_ptr<MessageFilterInterface>	MessageFilterPointer;

public:
	TCPSession(IOServiceThread &thread, MessageFilterPointer filter);
	~TCPSession();

public:
	TCPSessionID id() const
	{
		return session_id_;
	}

	SocketType& socket()
	{
		return socket_;
	}

	IOServiceThread& thread() const
	{
		return thread_;
	}

private:
	void init(TCPSessionID id);

	void set_session_id(TCPSessionID id)
	{
		session_id_ = id;
	}

private:
	TCPSessionID			session_id_;
	SocketType				socket_;
	IOServiceThread&		thread_;
	MessageFilterPointer	filter_;
};
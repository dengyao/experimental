#pragma once

#include <memory>
#include <asio/ip/tcp.hpp>
#include "types.h"
#include "net_message.h"

class IOServiceThread;
class MessageFilterInterface;

class TCPSession final : public std::enable_shared_from_this < TCPSession >
{
	friend class IOServiceThreadManager;

public:
	typedef asio::ip::tcp::socket					SocketType;
	typedef std::shared_ptr<NetMessage>				NetMessagePointer;
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

	void close();

private:
	void init(TCPSessionID id);

	void set_session_id(TCPSessionID id)
	{
		session_id_ = id;
	}

	void handle_read(asio::error_code, size_t bytes_transferred);

	void hanlde_write(asio::error_code, size_t bytes_transferred);

	void hanlde_close();

private:
	bool					closed_;
	int						num_handlers_;
	TCPSessionID			session_id_;
	SocketType				socket_;
	IOServiceThread&		thread_;
	MessageFilterPointer	filter_;
	std::vector<NetMessage>	messages_received_;
	std::vector<NetMessage>	messages_to_be_sent_;
};
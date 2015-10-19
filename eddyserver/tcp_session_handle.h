#pragma once

#include "tcp_session.h"
#include "types.h"

class IOServiceThreadManager;

class TCPSessionHandle : public std::enable_shared_from_this < TCPSessionHandle >
{
	friend class IOServiceThreadManager;

public:
	TCPSessionHandle();

	virtual ~TCPSessionHandle();

	void send(message_buffer &message);

	void close();

	void dispose();

	bool is_closed() const;

public:
	virtual void on_connect() = 0;

	virtual void on_message(message_buffer &message) = 0;

	virtual void on_close() = 0;

private:
	void init(TCPSessionID sid, ThreadID tid, IOServiceThreadManager* manager);

protected:
	TCPSessionHandle(const TCPSessionHandle &) = delete;
	TCPSessionHandle& operator= (const TCPSessionHandle &) = delete;

private:
	TCPSessionID				session_id_;
	ThreadID					session_thread_id_;
	IOServiceThreadManager*		io_thread_manager_;
	//message_vector				messages_to_be_sent_;
};
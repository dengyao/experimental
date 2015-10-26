#pragma once

#include "types.h"
#include "net_message.h"
#include "tcp_session.h"

class IOServiceThreadManager;


class TCPSessionHandle : public std::enable_shared_from_this < TCPSessionHandle >
{
	friend class IOServiceThreadManager;

public:
	TCPSessionHandle();

	virtual ~TCPSessionHandle();

	void send(NetMessage &message);

	void close();

	void dispose();

	bool is_closed() const;

public:
	virtual void on_connect() = 0;

	virtual void on_message(NetMessage &message) = 0;

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
};
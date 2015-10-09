#include "tcp_session.h"

#include <cassert>
#include "message_filter.h"
#include "io_service_thread.h"


const TCPSessionID kInvalidSessionID = 0;

TCPSession::TCPSession(IOServiceThread &thread, MessageFilterPointer filter)
	: thread_(thread)
	, filter_(filter)
	, id_(kInvalidSessionID)
	, socket_(thread.io_service())
{

}

TCPSession::~TCPSession()
{

}

void TCPSession::init(TCPSessionID id)
{
	assert(id != kInvalidSessionIDkInvalidSessionID);

	set_session_id(id);
	thread_.session_queue().add(shared_from_this());

	asio::ip::tcp::no_delay option(true);
	socket_.set_option(option);

	size_t size = filter_->bytes_wanna_read();
}
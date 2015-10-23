#include "tcp_session.h"

#include <cassert>
#include "id_generator.h"
#include "message_filter.h"
#include "io_service_thread.h"


TCPSession::TCPSession(IOServiceThread &thread, MessageFilterPointer filter)
	: thread_(thread)
	, filter_(filter)
	, socket_(thread.io_service())
	, session_id_(IDGenerator::kInvalidID)
{

}

TCPSession::~TCPSession()
{

}

void TCPSession::init(TCPSessionID id)
{
	assert(IDGenerator::kInvalidID != id);

	set_session_id(id);
	thread_.session_queue().add(shared_from_this());

	asio::ip::tcp::no_delay option(true);
	socket_.set_option(option);

	size_t size = filter_->bytes_wanna_read();
	if (size == MAXSIZE_T)
	{

	}
	else
	{
		NetMessage buffer;
		buffer.make_space(size);
		socket_.async_read_some(asio::buffer(buffer.peek(), size), , );
	}
}

void TCPSession::on_message(NetMessagePointer message, asio::error_code, size_t size)
{

}
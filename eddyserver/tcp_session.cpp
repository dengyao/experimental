#include "tcp_session.h"

#include <cassert>
#include <iostream>
#include "id_generator.h"
#include "message_filter.h"
#include "io_service_thread.h"
#include "tcp_session_handle.h"
#include "io_service_thread_manager.h"


TCPSession::TCPSession(IOServiceThread &thread, MessageFilterPointer filter)
	: closed_(false)
	, thread_(thread)
	, filter_(filter)
	, num_handlers_(0)
	, socket_(thread.io_service())
	, session_id_(IDGenerator::kInvalidID)
{

}

TCPSession::~TCPSession()
{

}

namespace helper
{
	typedef std::shared_ptr< std::vector<NetMessage> > NetMessageVecPointer;

	void SendMessageListToHandler(IOServiceThreadManager &manager, TCPSessionID id, NetMessageVecPointer messages)
	{
		std::shared_ptr<TCPSessionHandle> handler = manager.session_handler(id);
		if (handler == nullptr) return;

		for (auto &message : *messages)
		{
			handler->on_message(message);
		}
	}

	void PackMessageList(std::shared_ptr<TCPSession> session)
	{
		if (session->messages_received().empty()) return;

		NetMessageVecPointer messages = std::make_shared< std::vector<NetMessage> >(std::move(session->messages_received()));
		session->thread().manager().main_thread().post(std::bind(
			helper::SendMessageListToHandler, std::ref(session->thread().manager()), session->id(), messages));
	}

	void SendMessageListDirectly(std::shared_ptr<TCPSession> session)
	{
		std::shared_ptr<TCPSessionHandle> handler = session->thread().manager().session_handler(session->id());
		if (handler == nullptr) return;

		for (auto &message : session->messages_received())
		{
			handler->on_message(message);
		}
		session->messages_received().clear();
	}
}

void TCPSession::init(TCPSessionID id)
{
	assert(IDGenerator::kInvalidID != id);

	set_session_id(id);
	thread_.session_queue().add(shared_from_this());

	asio::ip::tcp::no_delay option(true);
	socket_.set_option(option);

	size_t bytes_wanna_read = filter_->bytes_wanna_read();
	if (bytes_wanna_read == 0) return;

	NetMessagePointer message = std::make_shared<NetMessage>(bytes_wanna_read);
	socket_.async_receive(asio::buffer(message->peek(), message->writable_bytes()),
		std::bind(&TCPSession::handle_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void TCPSession::hanlde_write(asio::error_code error_code, size_t bytes_transferred)
{

}

void TCPSession::handle_read(asio::error_code error_code, size_t bytes_transferred)
{
	if (error_code)
	{
		return;
	}
}

void TCPSession::hanlde_close()
{
	if (num_handlers_ > 0)
	{
		return;
	}

	thread_.manager().main_thread().post(
		std::bind(&IOServiceThreadManager::on_session_close, &thread_.manager(), id()));

	asio::error_code error_code;
	socket_.shutdown(asio::ip::tcp::socket::shutdown_both, error_code);
	if (error_code && error_code != asio::error::not_connected)
	{
		std::cerr << error_code.message() << std::endl;
	}

	socket_.close();
	thread_.session_queue().remove(id());
}

void TCPSession::close()
{
	if (closed_)
	{
		return;
	}
	closed_ = true;
	hanlde_close();
}
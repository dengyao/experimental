#include "tcp_session.h"

#include <cassert>
#include <iostream>
#include "id_generator.h"
#include "message_filter.h"
#include "io_service_thread.h"
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

//{
	/*void SendMessageListToHandler(TCPIOThreadManager& manager,
								  TCPSessionID id,
								  NetMessageVector* messages) {
		boost::shared_ptr<TCPSessionHandler> handler = manager.GetSessionHandler(id);

		if (handler == NULL)
			return;

		for_each(messages->begin(), messages->end(),
				 boost::bind(&TCPSessionHandler::OnMessage,
				 handler, _1));

		delete messages;
	}

	void PackMessageList(boost::shared_ptr<TCPSession> session) {
		if (session->messages_received().empty())
			return;

		NetMessageVector* messages(new NetMessageVector);
		messages->swap(session->messages_received());
		session->thread().manager().GetMainThread().Post(
			boost::bind(&SendMessageListToHandler,
			boost::ref(session->thread().manager()),
			session->id(),
			messages));
	}

	void SendMessageListDirectly(boost::shared_ptr<TCPSession> session) {
		boost::shared_ptr<TCPSessionHandler> handler =
			session->thread().manager().GetSessionHandler(session->id());

		if (handler == NULL)
			return;

		for_each(session->messages_received().begin(),
				 session->messages_received().end(),
				 boost::bind(&TCPSessionHandler::OnMessage,
				 handler, _1));

		session->messages_received().clear();
	}*/
//}

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
#include "tcp_client.h"
#include <cassert>
#include <iostream>
#include "tcp_session.h"
#include "io_service_thread.h"
#include "tcp_session_handle.h"
#include "io_service_thread_manager.h"

namespace eddy
{
	TCPClient::TCPClient(IOServiceThreadManager &io_thread_manager,
		const SessionHandlerCreator &handler_creator,
		const MessageFilterCreator &filter_creator)
		: io_thread_manager_(io_thread_manager)
		, session_handler_creator_(handler_creator)
		, message_filter_creator_(filter_creator)
	{

	}

	asio::io_service& TCPClient::io_service()
	{
		return io_thread_manager_.main_thread()->io_service();
	}

	void TCPClient::async_connect(asio::ip::tcp::endpoint &endpoint)
	{
		SessionPointer session_ptr = std::make_shared<TCPSession>(io_thread_manager_.thread(), message_filter_creator_());
		session_ptr->socket().async_connect(endpoint, std::bind(&TCPClient::handle_connect, this, session_ptr, std::placeholders::_1));
	}

	void TCPClient::connect(asio::ip::tcp::endpoint &endpoint, asio::error_code &ec)
	{
		SessionPointer session_ptr = std::make_shared<TCPSession>(io_thread_manager_.thread(), message_filter_creator_());
		session_ptr->socket().connect(endpoint, ec);
		handle_connect(session_ptr, ec);
	}

	void TCPClient::handle_connect(SessionPointer session_ptr, asio::error_code ec)
	{
		if (ec)
		{
			std::cerr << ec.message() << std::endl;
			assert(false);
			return;
		}
		SessionHandlerPointer handle_ptr = session_handler_creator_();
		io_thread_manager_.on_session_connect(session_ptr, handle_ptr);
	}
}
#include "tcp_server.h"

#include <cassert>
#include <iostream>
#include "tcp_session.h"
#include "io_service_thread.h"
#include "tcp_session_handle.h"
#include "io_service_thread_manager.h"


TCPServer::TCPServer(asio::ip::tcp::endpoint &endpoint,
					 IOServiceThreadManager &io_thread_manager,
					 const SessionHandlerCreator &handler_creator,
					 const MessageFilterCreator &filter_creator)
					 : io_thread_manager_(io_thread_manager)
					 , session_handler_creator_(handler_creator)
					 , message_filter_creator_(filter_creator)
					 , acceptor_(io_thread_manager.main_thread()->io_service(), endpoint)
{
	SessionPointer session_ptr = std::make_shared<TCPSession>(io_thread_manager_.thread(), message_filter_creator_());
	acceptor_.async_accept(session_ptr->socket(), std::bind(&TCPServer::handle_accept, this, session_ptr, std::placeholders::_1));
}

asio::io_service& TCPServer::io_service()
{
	return io_thread_manager_.main_thread()->io_service();
}

void TCPServer::handle_accept(SessionPointer session_ptr, asio::error_code error)
{
	if (error)
	{
		std::cerr << error.message() << std::endl;
		assert(false);
		return;
	}

	SessionHandlerPointer handle_ptr = session_handler_creator_();
	io_thread_manager_.on_session_connect(session_ptr, handle_ptr);

	SessionPointer new_session_ptr = std::make_shared<TCPSession>(io_thread_manager_.thread(), message_filter_creator_());
	acceptor_.async_accept(new_session_ptr->socket(), std::bind(&TCPServer::handle_accept, this, new_session_ptr, std::placeholders::_1));
}
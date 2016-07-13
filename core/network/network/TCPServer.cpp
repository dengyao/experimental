#include "TCPServer.h"
#include <cassert>
#include <iostream>
#include "TCPSession.h"
#include "IOServiceThread.h"
#include "TCPSessionHandler.h"
#include "IOServiceThreadManager.h"

namespace network
{
	TCPServer::TCPServer(asio::ip::tcp::endpoint &endpoint,
		IOServiceThreadManager &io_thread_manager,
		const SessionHandlerCreator &handler_creator,
		const MessageFilterCreator &filter_creator,
		uint64_t timeout)
		: io_thread_manager_(io_thread_manager)
		, session_handler_creator_(handler_creator)
		, message_filter_creator_(filter_creator)
		, acceptor_(io_thread_manager.MainThread()->IOService(), endpoint)
	{
		io_thread_manager.SetSessionTimeout(timeout);
		MessageFilterPointer filter_ptr = message_filter_creator_();
		SessionPointer session_ptr = std::make_shared<TCPSession>(io_thread_manager_.Thread(), filter_ptr);
		acceptor_.async_accept(session_ptr->Socket(), std::bind(&TCPServer::HandleAccept, this, session_ptr, std::placeholders::_1));
	}

	asio::io_service& TCPServer::IOService()
	{
		return io_thread_manager_.MainThread()->IOService();
	}

	void TCPServer::HandleAccept(SessionPointer session_ptr, asio::error_code error)
	{
		if (error)
		{
			std::cerr << error.message() << std::endl;
			assert(false);
			return;
		}

		SessionHandlePointer handle_ptr = session_handler_creator_();
		io_thread_manager_.OnSessionConnect(session_ptr, handle_ptr);

		MessageFilterPointer filter_ptr = message_filter_creator_();
		SessionPointer new_session_ptr = std::make_shared<TCPSession>(io_thread_manager_.Thread(), filter_ptr);
		acceptor_.async_accept(new_session_ptr->Socket(), std::bind(&TCPServer::HandleAccept, this, new_session_ptr, std::placeholders::_1));
	}
}
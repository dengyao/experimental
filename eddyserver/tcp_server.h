#pragma once

#include <functional>
#include <asio.hpp>
#include "types.h"


namespace eddy
{
	class IOServiceThreadManager;

	class TCPServer
	{
	public:
		typedef std::function<SessionHandlerPointer()>	SessionHandlerCreator;
		typedef std::function<MessageFilterPointer()>	MessageFilterCreator;

	public:
		TCPServer(asio::ip::tcp::endpoint &endpoint,
				  IOServiceThreadManager &io_thread_manager,
				  const SessionHandlerCreator &handler_creator,
				  const MessageFilterCreator &filter_creator);

		asio::io_service& io_service();

	private:
		void handle_accept(SessionPointer session_ptr, asio::error_code error);

	private:
		asio::ip::tcp::acceptor		acceptor_;
		IOServiceThreadManager&		io_thread_manager_;
		SessionHandlerCreator		session_handler_creator_;
		MessageFilterCreator		message_filter_creator_;
	};
}
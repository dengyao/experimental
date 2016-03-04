#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include <asio.hpp>
#include "types.h"

namespace eddy
{
	class IOServiceThreadManager;

	class TCPServer
	{
	public:
		TCPServer(asio::ip::tcp::endpoint &endpoint,
				  IOServiceThreadManager &io_thread_manager,
				  const SessionHandlerCreator &handler_creator,
				  const MessageFilterCreator &filter_creator);

		asio::io_service& io_service();

	private:
		void handle_accept(SessionPointer session_ptr, asio::error_code error);

	private:
		asio::ip::tcp::acceptor			acceptor_;
		IOServiceThreadManager&			io_thread_manager_;
		const SessionHandlerCreator&	session_handler_creator_;
		const MessageFilterCreator&		message_filter_creator_;
	};
}

#endif
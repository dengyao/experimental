#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__

#include <asio.hpp>
#include "types.h"

namespace eddy
{
	class IOServiceThreadManager;

	class TCPClient
	{
	public:
		TCPClient(IOServiceThreadManager &io_thread_manager,
			const SessionHandlerCreator &handler_creator,
			const MessageFilterCreator &filter_creator);

		asio::io_service& io_service();

		void async_connect(asio::ip::tcp::endpoint &endpoint);

		void connect(asio::ip::tcp::endpoint &endpoint, asio::error_code &error_code);

	private:
		void handle_connect(SessionPointer session_ptr, asio::error_code error_code);

	private:
		IOServiceThreadManager&			io_thread_manager_;
		const SessionHandlerCreator&	session_handler_creator_;
		const MessageFilterCreator&		message_filter_creator_;
	};
}

#endif
#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include <asio.hpp>
#include "Types.h"

namespace eddy
{
	class IOServiceThreadManager;

	class TCPServer
	{
	public:
		TCPServer(asio::ip::tcp::endpoint &endpoint,
			IOServiceThreadManager &io_thread_manager,
			const SessionHandlerCreator &handler_creator,
			const MessageFilterCreator &filter_creator,
			uint32_t timeout = 0);

		asio::io_service& IOService();

	protected:
		TCPServer(const TCPServer&) = delete;
		TCPServer& operator= (const TCPServer&) = delete;

	private:
		void HandleAccept(SessionPointer session_ptr, asio::error_code error);

	private:
		asio::ip::tcp::acceptor			acceptor_;
		IOServiceThreadManager&			io_thread_manager_;
		const SessionHandlerCreator&	session_handler_creator_;
		const MessageFilterCreator&		message_filter_creator_;
	};
}

#endif
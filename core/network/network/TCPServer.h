#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include <asio.hpp>
#include "Types.h"

namespace network
{
	class IOServiceThreadManager;

	class TCPServer
	{
		TCPServer(const TCPServer&) = delete;
		TCPServer& operator= (const TCPServer&) = delete;

	public:
		TCPServer(asio::ip::tcp::endpoint &endpoint,
			IOServiceThreadManager &io_thread_manager,
			const SessionHandlerCreator &handler_creator,
			const MessageFilterCreator &filter_creator,
			uint32_t keep_alive = 0);

		~TCPServer() = default;

	public:
		asio::io_service& IOService();

		asio::ip::tcp::endpoint LocalEndpoint() const;

	private:
		void HandleAccept(SessionPointer session_ptr, asio::error_code error);

	private:
		const uint32_t			keep_alive_;
		asio::ip::tcp::acceptor	acceptor_;
		IOServiceThreadManager&	io_thread_manager_;
		SessionHandlerCreator	session_handler_creator_;
		MessageFilterCreator	message_filter_creator_;
	};
}

#endif
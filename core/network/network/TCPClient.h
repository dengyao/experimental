#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__

#include <asio.hpp>
#include "Types.h"

namespace network
{
	class IOServiceThreadManager;

	class TCPClient final
	{
	public:
		TCPClient(IOServiceThreadManager &io_thread_manager,
			const SessionHandlerCreator &handler_creator,
			const MessageFilterCreator &filter_creator);

		~TCPClient() = default;

	public:
		asio::io_service& IOService();

		void Connect(asio::ip::tcp::endpoint &endpoint, asio::error_code &error_code);

		void AsyncConnect(asio::ip::tcp::endpoint &endpoint, const std::function<void(asio::error_code)> &connect_handler);

	protected:
		TCPClient(const TCPClient&) = delete;
		TCPClient& operator= (const TCPClient&) = delete;

	private:
		void HandleConnect(SessionPointer session_ptr, asio::error_code error_code);

		void HandleAsyncConnect(SessionPointer session_ptr, std::function<void(asio::error_code)> connect_handler, asio::error_code error_code);

	private:
		IOServiceThreadManager&			io_thread_manager_;
		SessionHandlerCreator			session_handler_creator_;
		MessageFilterCreator			message_filter_creator_;
	};
}

#endif
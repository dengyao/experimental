#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__

#include <asio.hpp>
#include "Types.h"

namespace eddy
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

		void AsyncConnect(asio::ip::tcp::endpoint &endpoint);

		void Connect(asio::ip::tcp::endpoint &endpoint, asio::error_code &error_code);

	protected:
		TCPClient(const TCPClient&) = delete;
		TCPClient& operator= (const TCPClient&) = delete;

	private:
		void HandleConnect(SessionPointer session_ptr, asio::error_code error_code);

	private:
		IOServiceThreadManager&			io_thread_manager_;
		const SessionHandlerCreator&	session_handler_creator_;
		const MessageFilterCreator&		message_filter_creator_;
	};
}

#endif
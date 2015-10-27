#pragma once

#include <memory>
#include <functional>
#include <asio.hpp>


namespace eddy
{
	class TCPSession;
	class TCPSessionHandle;
	class MessageFilterInterface;
	class IOServiceThreadManager;

	class TCPServer
	{
	public:
		typedef std::shared_ptr<TCPSession>				SessionPointer;
		typedef std::shared_ptr<TCPSessionHandle>		SessionHandlerPointer;
		typedef std::function<SessionHandlerPointer()>	SessionHandlerCreator;
		typedef std::shared_ptr<MessageFilterInterface>	MessageFilterPointer;
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
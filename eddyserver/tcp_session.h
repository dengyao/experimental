#pragma once

#include <memory>
#include <asio/ip/tcp.hpp>
#include "types.h"
#include "net_message.h"


namespace eddy
{
	class TCPSession : public std::enable_shared_from_this < TCPSession >
	{
		friend class IOServiceThreadManager;
		typedef asio::ip::tcp::socket SocketType;

	public:
		TCPSession(ThreadPointer thread_ptr, MessageFilterPointer filter);
		~TCPSession();

	public:
		TCPSessionID id() const
		{
			return session_id_;
		}

		SocketType& socket()
		{
			return socket_;
		}

		ThreadPointer thread()
		{
			return thread_;
		}

		void post_message_list(std::vector<NetMessage> &messages);

		std::vector<NetMessage>& messages_received()
		{
			return messages_received_;
		}

		void close();

	private:
		void init(TCPSessionID id);

		void set_session_id(TCPSessionID id)
		{
			session_id_ = id;
		}

		void handle_read(asio::error_code error, size_t bytes_transferred);

		void hanlde_write(asio::error_code error, size_t bytes_transferred);

		void hanlde_close();

	private:
		bool					closed_;
		int						num_handlers_;
		TCPSessionID			session_id_;
		SocketType				socket_;
		ThreadPointer			thread_;
		MessageFilterPointer	filter_;
		std::vector<char>		buffer_receiving_;
		std::vector<char>		buffer_sending_;
		std::vector<char>		buffer_to_be_sent_;
		NetMessageVector		messages_received_;
		NetMessageVector		messages_to_be_sent_;
	};
}
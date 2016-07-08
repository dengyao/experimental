#ifndef __TCP_SESSION_H__
#define __TCP_SESSION_H__

#include <asio/ip/tcp.hpp>
#include "EddyTypes.h"
#include "NetMessage.h"

namespace eddy
{
	class TCPSession final : public std::enable_shared_from_this < TCPSession >
	{
		friend class IOServiceThreadManager;
		typedef asio::ip::tcp::socket SocketType;

	public:
		TCPSession(ThreadPointer &thread_ptr, MessageFilterPointer &filter);
		~TCPSession() = default;

	public:
		TCPSessionID ID() const
		{
			return session_id_;
		}

		SocketType& Socket()
		{
			return socket_;
		}

		ThreadPointer& Thread()
		{
			return thread_;
		}

		const TimePoint& LastActivity() const
		{
			return last_activity_;
		}

		void PostMessageList(const std::vector<NetMessage> &messages);

		std::vector<NetMessage>& MessagesReceived()
		{
			return messages_received_;
		}

		void Close();

	protected:
		TCPSession(const TCPSession&) = delete;
		TCPSession& operator= (const TCPSession&) = delete;

	private:
		void Init(TCPSessionID id);

		void UpdateLastActivity();

		void SetSessionID(TCPSessionID id)
		{
			session_id_ = id;
		}

		void HandleRead(asio::error_code error, size_t bytes_transferred);

		void HanldeWrite(asio::error_code error, size_t bytes_transferred);

		void HanldeClose();

	private:
		bool					closed_;
		int						num_handlers_;
		TCPSessionID			session_id_;
		SocketType				socket_;
		ThreadPointer			thread_;
		MessageFilterPointer	filter_;
		TimePoint				last_activity_;
		std::vector<uint8_t>	buffer_receiving_;
		std::vector<uint8_t>	buffer_sending_;
		std::vector<uint8_t>	buffer_to_be_sent_;
		NetMessageVector		messages_received_;
	};
}

#endif
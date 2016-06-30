#ifndef __TCP_SESSION_HANDLE_H__
#define __TCP_SESSION_HANDLE_H__

#include "Types.h"
#include "NetMessage.h"
#include "TCPSession.h"
#include "IOServiceThread.h"
#include "IOServiceThreadManager.h"

namespace eddy
{
	class TCPSessionHandler : public std::enable_shared_from_this < TCPSessionHandler >
	{
		friend class IOServiceThreadManager;

	public:
		TCPSessionHandler();
		virtual ~TCPSessionHandler() = default;

	public:
		virtual void OnConnect() = 0;

		virtual void OnMessage(NetMessage &message) = 0;

		virtual void OnClose() = 0;

	public:
		void Send(const NetMessage &message);

		void Close();

		void Dispose();

		bool IsClosed() const;

		IOThreadID ThreadID() const
		{
			return thread_id_;
		}

		TCPSessionID SessionID() const
		{
			return session_id_;
		}

		IOServiceThreadManager* ThreadManager()
		{
			return io_thread_manager_;
		}

		std::vector<NetMessage>& MessagesToBeSent()
		{
			return messages_to_be_sent_;
		}

	protected:
		TCPSessionHandler(const TCPSessionHandler&) = delete;
		TCPSessionHandler& operator= (const TCPSessionHandler&) = delete;

	private:
		void Init(TCPSessionID sid, IOThreadID tid, IOServiceThreadManager *manager);

	private:
		TCPSessionID				session_id_;
		IOThreadID					thread_id_;
		IOServiceThreadManager*		io_thread_manager_;
		std::vector<NetMessage>		messages_to_be_sent_;
	};
}

#endif
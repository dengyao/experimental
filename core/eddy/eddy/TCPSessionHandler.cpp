#include "TCPSessionHandler.h"
#include "IOServiceThread.h"
#include "IOServiceThreadManager.h"

namespace eddy
{
	namespace
	{
		typedef std::shared_ptr< std::vector<NetMessage> > NetMessageVecPointer;

		void CloseSession(ThreadPointer thread_ptr, TCPSessionID id)
		{
			SessionPointer session_ptr = thread_ptr->SessionQueue().Get(id);
			if (session_ptr != nullptr)
			{
				session_ptr->Close();
			}
		}

		void SendMessageListToSession(ThreadPointer thread_ptr, TCPSessionID id, NetMessageVecPointer messages)
		{
			SessionPointer session_ptr = thread_ptr->SessionQueue().Get(id);
			if (session_ptr != nullptr)
			{
				session_ptr->PostMessageList(*messages);
			}
		}

		void PackMessageList(SessionHandlePointer session_handle_ptr)
		{
			if (session_handle_ptr->MessagesToBeSent().empty())
			{
				return;
			}

			ThreadPointer thread_ptr = session_handle_ptr->ThreadManager()->Thread(session_handle_ptr->ThreadID());
			if (thread_ptr != nullptr)
			{
				NetMessageVecPointer messages = std::make_shared< std::vector<NetMessage> >(std::move(session_handle_ptr->MessagesToBeSent()));
				thread_ptr->Post(std::bind(SendMessageListToSession, thread_ptr, session_handle_ptr->SessionID(), messages));
			}
		}

		void SendMessageListDirectly(SessionHandlePointer session_handle_ptr)
		{
			ThreadPointer thread_ptr = session_handle_ptr->ThreadManager()->Thread(session_handle_ptr->ThreadID());
			if (thread_ptr != nullptr)
			{
				SessionPointer session_ptr = thread_ptr->SessionQueue().Get(session_handle_ptr->SessionID());
				if (session_ptr != nullptr)
				{
					session_ptr->PostMessageList(session_handle_ptr->MessagesToBeSent());
					session_handle_ptr->MessagesToBeSent().clear();
				}
			}
		}
	}

	TCPSessionHandler::TCPSessionHandler()
		: session_id_(IDGenerator::kInvalidID)
	{
	}

	void TCPSessionHandler::Init(TCPSessionID sid, IOThreadID tid, IOServiceThreadManager* manager)
	{
		thread_id_ = tid;
		session_id_ = sid;
		io_thread_manager_ = manager;
	}

	void TCPSessionHandler::Dispose()
	{
		session_id_ = IDGenerator::kInvalidID;
	}

	bool TCPSessionHandler::IsClosed() const
	{
		return IDGenerator::kInvalidID == session_id_;
	}

	void TCPSessionHandler::Close()
	{
		if (IsClosed())
		{
			return;
		}

		PackMessageList(shared_from_this());

		ThreadPointer thread_ptr = ThreadManager()->Thread(thread_id_);
		if (thread_ptr != nullptr)
		{
			thread_ptr->Post(std::bind(CloseSession, thread_ptr, session_id_));
		}
	}

	void TCPSessionHandler::Send(const NetMessage &message)
	{
		if (IsClosed())
		{
			return;
		}

		if (message.Empty())
		{
			return;
		}

		bool wanna_send = messages_to_be_sent_.empty();
		messages_to_be_sent_.push_back(message);

		if (wanna_send)
		{
			if (thread_id_ == io_thread_manager_->MainThread()->ID())
			{
				io_thread_manager_->MainThread()->Post(std::bind(SendMessageListDirectly, shared_from_this()));
			}
			else
			{
				io_thread_manager_->MainThread()->Post(std::bind(PackMessageList, shared_from_this()));
			}
		}
	}
}
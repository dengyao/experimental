#include "tcp_session_handle.h"

#include "id_generator.h"
#include "io_service_thread.h"
#include "io_service_thread_manager.h"


namespace eddy
{
	namespace
	{
		typedef std::shared_ptr< std::vector<NetMessage> > NetMessageVecPointer;

		void CloseSession(ThreadPointer thread_ptr, TCPSessionID id)
		{
			SessionPointer session_ptr = thread_ptr->session_queue().get(id);
			if (session_ptr != nullptr)
			{
				session_ptr->close();
			}
		}

		void SendMessageListToSession(ThreadPointer thread_ptr, TCPSessionID id, NetMessageVecPointer messages)
		{
			SessionPointer session_ptr = thread_ptr->session_queue().get(id);
			if (session_ptr != nullptr)
			{
				session_ptr->post_message_list(*messages);
			}
		}

		void PackMessageList(SessionHandlerPointer session_handle_ptr)
		{
			if (session_handle_ptr->messages_to_be_sent().empty()) return;

			ThreadPointer thread_ptr = session_handle_ptr->thread_manager()->thread(session_handle_ptr->thread_id());
			if (thread_ptr != nullptr)
			{
				NetMessageVecPointer messages = std::make_shared< std::vector<NetMessage> >(std::move(session_handle_ptr->messages_to_be_sent()));
				thread_ptr->post(std::bind(SendMessageListToSession, thread_ptr, session_handle_ptr->session_id(), messages));
			}
		}

		void SendMessageListDirectly(SessionHandlerPointer session_handle_ptr)
		{
			ThreadPointer thread_ptr = session_handle_ptr->thread_manager()->thread(session_handle_ptr->thread_id());
			if (thread_ptr != nullptr)
			{
				SessionPointer session_ptr = thread_ptr->session_queue().get(session_handle_ptr->session_id());
				if (session_ptr != nullptr)
				{
					session_ptr->post_message_list(session_handle_ptr->messages_to_be_sent());
					session_handle_ptr->messages_to_be_sent().clear();
				}
			}
		}
	}

	TCPSessionHandle::TCPSessionHandle()
		: session_id_(IDGenerator::kInvalidID)
	{

	}

	TCPSessionHandle::~TCPSessionHandle()
	{

	}

	void TCPSessionHandle::init(TCPSessionID sid, ThreadID tid, IOServiceThreadManager* manager)
	{
		thread_id_ = tid;
		session_id_ = sid;
		io_thread_manager_ = manager;
	}

	void TCPSessionHandle::dispose()
	{
		session_id_ = IDGenerator::kInvalidID;
	}

	bool TCPSessionHandle::is_closed() const
	{
		return IDGenerator::kInvalidID == session_id_;
	}

	void TCPSessionHandle::close()
	{
		if (is_closed()) return;

		PackMessageList(shared_from_this());

		ThreadPointer thread_ptr = thread_manager()->thread(thread_id_);
		if (thread_ptr != nullptr)
		{
			thread_ptr->post(std::bind(CloseSession, thread_ptr, session_id_));
		}
	}

	void TCPSessionHandle::send(NetMessage &message)
	{
		if (is_closed()) return;

		if (message.empty()) return;

		bool wanna_send = messages_to_be_sent_.empty();
		messages_to_be_sent_.push_back(message);

		if (wanna_send)
		{
			if (thread_id_ == io_thread_manager_->main_thread()->id())
			{
				io_thread_manager_->main_thread()->post(std::bind(SendMessageListDirectly, shared_from_this()));
			}
			else
			{
				io_thread_manager_->main_thread()->post(std::bind(PackMessageList, shared_from_this()));
			}
		}
	}
}
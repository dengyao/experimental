#include "tcp_session_handle.h"

#include "id_generator.h"
#include "io_service_thread.h"
#include "io_service_thread_manager.h"


namespace helper
{
	typedef std::shared_ptr<TCPSession>			SessionPointer;
	typedef std::shared_ptr<IOServiceThread>	ThreadPointer;

	void CloseSession(ThreadPointer thread_ptr, TCPSessionID id)
	{
		SessionPointer session_ptr = thread_ptr->session_queue().get(id);
		if (session_ptr != nullptr)
		{
			session_ptr->close();
		}
	}

	void SendMessageListToSession()
	{

	}

	void PackMessageList()
	{

	}

	void SendMessageListDirectly()
	{

	}
}

TCPSessionHandle::TCPSessionHandle()
	: session_id_(IDGenerator::kInvalidID)
{

}

TCPSessionHandle::~TCPSessionHandle()
{

}

void TCPSessionHandle::dispose()
{
	session_id_ = IDGenerator::kInvalidID;
}

bool TCPSessionHandle::is_closed() const
{
	return IDGenerator::kInvalidID == session_id_;
}

void TCPSessionHandle::init(TCPSessionID sid, ThreadID tid, IOServiceThreadManager* manager)
{
	session_id_ = sid;
	session_thread_id_ = tid;
	io_thread_manager_ = manager;
}

void TCPSessionHandle::close()
{
	if (is_closed()) return;

	ThreadPointer thread_ptr = io_thread_manager_->thread(session_thread_id_);
	if (thread_ptr != nullptr)
	{
		thread_ptr->post(std::bind(helper::CloseSession, thread_ptr, session_id_));
	}
}

void TCPSessionHandle::send(NetMessage &message)
{
	if (is_closed()) return;

	if (message.readable_bytes() == 0) return;

	bool wanna_send = messages_to_be_sent_.empty();
	messages_to_be_sent_.push_back(message);

	if (wanna_send)
	{
		if (session_thread_id_ == io_thread_manager_->main_thread()->id())
		{
			ThreadPointer thread_ptr = io_thread_manager_->thread(session_thread_id_);
			if (thread_ptr != nullptr)
			{
				SessionPointer session_ptr = thread_ptr->session_queue().get(session_id_);
				if (session_ptr != nullptr)
				{

				}
			}
		}
		else
		{

		}
	}
}
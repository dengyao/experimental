#include "tcp_session_handle.h"

#include "id_generator.h"
#include "io_service_thread.h"
#include "io_service_thread_manager.h"


namespace helper
{
	/*void CloseSession(ThreadPointer td, TCPSessionID id)
	{
	std::shared_ptr<TCPSession> session = td->session_queue().get(id);
	if (session != nullptr)
	{
	session->close();
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

	}*/
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

	std::shared_ptr<IOServiceThread> td = io_thread_manager_->thread(session_thread_id_);
	if (td != nullptr)
	{
		//td->post(std::bind(helper::CloseSession, td, session_id_));
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
			std::shared_ptr<IOServiceThread> td = io_thread_manager_->thread(session_thread_id_);
			if (td != nullptr)
			{
				std::shared_ptr<TCPSession> session = td->session_queue().get(session_id_);
				if (session != nullptr)
				{

				}
			}
		}
		else
		{

		}
	}
}
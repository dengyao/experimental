#include "tcp_session_handle.h"

#include "id_generator.h"
#include "io_service_thread.h"
#include "io_service_thread_manager.h"


namespace helper
{
	void CloseSession(IOServiceThread *td, TCPSessionID id)
	{
		std::shared_ptr<TCPSession> session;
		if (td->session_queue().get(id, session))
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

	IOServiceThread *td = nullptr;
	if (io_thread_manager_->thread(session_thread_id_, td))
	{
		td->post(std::bind(helper::CloseSession, td, session_id_));
	}
}

void TCPSessionHandle::send(NetMessage &message)
{

}
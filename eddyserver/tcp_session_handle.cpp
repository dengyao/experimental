#include "tcp_session_handle.h"
#include "id_generator.h"


TCPSessionHandle::TCPSessionHandle()
	: session_id_(IDGenerator::kInvalidID)
{

}

TCPSessionHandle::~TCPSessionHandle()
{

}

void TCPSessionHandle::send(NetMessage &message)
{

}

void TCPSessionHandle::close()
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
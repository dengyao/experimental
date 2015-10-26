#include "tcp_session_queue.h"

#include "tcp_session.h"


TCPSessionQueue::TCPSessionQueue()
	: load_count_(0)
{

}

TCPSessionQueue::~TCPSessionQueue()
{

}

size_t TCPSessionQueue::size() const
{
	return load_count_;
}

void TCPSessionQueue::add(SessionPointer session_ptr)
{
	if (session_queue_.count(session_ptr->id()) == 0)
	{
		++load_count_;
	}
	session_queue_.insert(std::make_pair(session_ptr->id(), session_ptr));
}

TCPSessionQueue::SessionPointer TCPSessionQueue::get(TCPSessionID id)
{
	std::unordered_map<TCPSessionID, SessionPointer>::iterator itr = session_queue_.find(id);
	if (itr == session_queue_.end())
	{
		return nullptr;
	}
	return itr->second;
}

void TCPSessionQueue::remove(TCPSessionID id)
{
	if (session_queue_.count(id) == 1)
	{
		--load_count_;
	}
	session_queue_.erase(id);
}

void TCPSessionQueue::clear()
{
	session_queue_.clear();
}
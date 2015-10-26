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

void TCPSessionQueue::add(SessionPointer session)
{
	if (session_queue_.count(session->id()) == 0)
	{
		++load_count_;
	}
	session_queue_.insert(std::make_pair(session->id(), session));
}

bool TCPSessionQueue::get(TCPSessionID id, SessionPointer &session)
{
	std::unordered_map<TCPSessionID, SessionPointer>::iterator itr = session_queue_.find(id);
	if (itr == session_queue_.end())
	{
		return false;
	}
	session = itr->second;
	return true;
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
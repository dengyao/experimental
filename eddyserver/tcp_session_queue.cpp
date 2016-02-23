#include "tcp_session_queue.h"
#include "tcp_session.h"

namespace eddy
{
	TCPSessionQueue::TCPSessionQueue()
	{

	}

	TCPSessionQueue::~TCPSessionQueue()
	{

	}

	size_t TCPSessionQueue::size() const
	{
		return session_queue_.size();
	}

	void TCPSessionQueue::add(SessionPointer session_ptr)
	{
		session_queue_.insert(std::make_pair(session_ptr->id(), session_ptr));
	}

	SessionPointer TCPSessionQueue::get(TCPSessionID id)
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
		session_queue_.erase(id);
	}

	void TCPSessionQueue::clear()
	{
		session_queue_.clear();
	}
}
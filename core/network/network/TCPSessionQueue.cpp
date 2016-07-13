#include "TCPSessionQueue.h"
#include "TCPSession.h"

namespace network
{
	TCPSessionQueue::TCPSessionQueue()
	{

	}

	TCPSessionQueue::~TCPSessionQueue()
	{

	}

	size_t TCPSessionQueue::Size() const
	{
		return session_queue_.size();
	}

	void TCPSessionQueue::Add(SessionPointer session_ptr)
	{
		session_queue_.insert(std::make_pair(session_ptr->ID(), session_ptr));
	}

	SessionPointer TCPSessionQueue::Get(TCPSessionID id)
	{
		std::unordered_map<TCPSessionID, SessionPointer>::iterator found = session_queue_.find(id);
		if (found != session_queue_.end())
		{
			return found->second;
		}
		return SessionPointer();
	}

	void TCPSessionQueue::Remove(TCPSessionID id)
	{
		session_queue_.erase(id);
	}

	void TCPSessionQueue::Clear()
	{
		session_queue_.clear();
	}

	void TCPSessionQueue::Foreach(const std::function<void(const SessionPointer &session)> &func)
	{
		for (const auto &item : session_queue_)
		{
			func(item.second);
		}
	}
}
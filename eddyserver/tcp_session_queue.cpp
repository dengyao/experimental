#include "tcp_session_queue.h"

#include "tcp_session.h"


size_t TCPSessionQueue::size() const
{
	std::lock_guard<std::mutex> lock(locker_);
	return session_queue_.size();
}

void TCPSessionQueue::add(SessionPointer session)
{
	std::lock_guard<std::mutex> lock(locker_);
	session_queue_.insert(std::make_pair(session->id(), session));
}

void TCPSessionQueue::remove(TCPSessionID id)
{
	std::lock_guard<std::mutex> lock(locker_);
	session_queue_.erase(id);
}

void TCPSessionQueue::clear()
{
	std::lock_guard<std::mutex> lock(locker_);
	session_queue_.clear();
}
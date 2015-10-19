#pragma once

#include <mutex>
#include <memory>
#include <unordered_map>
#include "types.h"

class TCPSession;

class TCPSessionQueue final
{
	typedef std::shared_ptr<TCPSession> SessionPointer;

public:
	TCPSessionQueue() = default;
	~TCPSessionQueue() = default;

public:
	size_t size() const;

	void add(SessionPointer session);

	void remove(TCPSessionID id);

	void clear();

private:
	TCPSessionQueue(const TCPSessionQueue&) = delete;
	TCPSessionQueue& operator= (const TCPSessionQueue&) = delete;

private:
	mutable std::mutex									locker_;
	std::unordered_map<TCPSessionID, SessionPointer>	session_queue_;
};
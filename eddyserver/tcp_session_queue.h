#pragma once

#include <memory>
#include <atomic>
#include <unordered_map>
#include "types.h"


namespace eddy
{
	class TCPSessionQueue
	{
	public:
		TCPSessionQueue();
		~TCPSessionQueue();

	public:
		size_t size() const;

		void add(SessionPointer session_ptr);

		SessionPointer get(TCPSessionID id);

		void remove(TCPSessionID id);

		void clear();

	private:
		TCPSessionQueue(const TCPSessionQueue&) = delete;
		TCPSessionQueue& operator= (const TCPSessionQueue&) = delete;

	private:
		std::atomic<size_t>									load_count_;
		std::unordered_map<TCPSessionID, SessionPointer>	session_queue_;
	};
}
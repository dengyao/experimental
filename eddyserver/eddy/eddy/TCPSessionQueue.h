#ifndef __TCP_SESSION_QUEUE_H__
#define __TCP_SESSION_QUEUE_H__

#include <unordered_map>
#include "Types.h"

namespace eddy
{
	class TCPSessionQueue
	{
	public:
		TCPSessionQueue();
		~TCPSessionQueue();

	public:
		size_t Size() const;

		void Add(SessionPointer session_ptr);

		SessionPointer Get(TCPSessionID id);

		void Remove(TCPSessionID id);

		void Clear();

	private:
		TCPSessionQueue(const TCPSessionQueue&) = delete;
		TCPSessionQueue& operator= (const TCPSessionQueue&) = delete;

	private:
		std::unordered_map<TCPSessionID, SessionPointer> session_queue_;
	};
}

#endif
#ifndef __IO_SERVICE_THREAD_MANAGER_H__
#define __IO_SERVICE_THREAD_MANAGER_H__

#include <map>
#include <vector>
#include <unordered_map>
#include "Types.h"
#include "IDGenerator.h"

namespace network
{
	class IOServiceThreadManager
	{
		typedef std::unordered_map<TCPSessionID, SessionHandlePointer > SessionHandlerMap;
		IOServiceThreadManager(const IOServiceThreadManager&) = delete;
		IOServiceThreadManager& operator=(const IOServiceThreadManager&) = delete;

	public:
		explicit IOServiceThreadManager(size_t thread_num = 1);
		~IOServiceThreadManager();

	public:
		void Run();

		void Stop();

		ThreadPointer& Thread();

		ThreadPointer Thread(IOThreadID id);

		ThreadPointer& MainThread();

		void OnSessionConnect(SessionPointer &session_ptr, SessionHandlePointer &handler);

		void OnSessionClose(TCPSessionID id);

		size_t SessionNumber() const;

		SessionHandlePointer SessionHandler(TCPSessionID id) const;

	private:
		std::vector<ThreadPointer>	threads_;
		std::vector<size_t>			thread_load_;
		SessionHandlerMap			session_handler_map_;
		IDGenerator					id_generator_;
	};
}

#endif
#ifndef __IO_SERVICE_THREAD_MANAGER_H__
#define __IO_SERVICE_THREAD_MANAGER_H__

#include <map>
#include <vector>
#include <unordered_map>
#include "Types.h"
#include "IDGenerator.h"

namespace eddy
{
	class IOServiceThreadManager
	{
		typedef std::unordered_map<TCPSessionID, SessionHandlerPointer > SessionHandlerMap;

	public:
		explicit IOServiceThreadManager(size_t thread_num = 1);

		~IOServiceThreadManager();

	public:
		void Run();

		void Stop();

		ThreadPointer& Thread();

		ThreadPointer Thread(IOThreadID id);

		ThreadPointer& MainThread();

		void OnSessionConnect(SessionPointer &session_ptr, SessionHandlerPointer &handler);

		void OnSessionClose(TCPSessionID id);

		SessionHandlerPointer SessionHandler(TCPSessionID id) const;

		void SetSessionTimeout(uint64_t timeout);

	protected:
		IOServiceThreadManager(const IOServiceThreadManager&) = delete;
		IOServiceThreadManager& operator=(const IOServiceThreadManager&) = delete;

	private:
		std::vector<ThreadPointer>	threads_;
		std::vector<size_t>			thread_load_;
		SessionHandlerMap			session_handler_map_;
		IDGenerator					id_generator_;
	};
}

#endif
#ifndef __IO_SERVICE_THREAD_H__
#define __IO_SERVICE_THREAD_H__

#include <thread>
#include <asio/io_service.hpp>
#include "TCPSessionQueue.h"

namespace eddy
{
	class IOServiceThreadManager;

	class IOServiceThread : public std::enable_shared_from_this < IOServiceThread >
	{
		friend class IOServiceThreadManager;

	public:
		IOServiceThread(IOThreadID id, IOServiceThreadManager &manager);
		~IOServiceThread();

	public:
		void RunThread();

		void Join();

		void Stop();

		template <typename CompletionHandler>
		void Post(ASIO_MOVE_ARG(CompletionHandler) handler)
		{
			io_service_.post(handler);
		}

	public:
		IOThreadID ID() const
		{
			return thread_id_;
		}

		asio::io_service& IOService()
		{
			return io_service_;
		}

		IOServiceThreadManager& ThreadManager()
		{
			return manager_;
		}

		TCPSessionQueue& SessionQueue()
		{
			return session_queue_;
		}

	private:
		void Run();

	private:
		IOServiceThread(const IOServiceThread&) = delete;
		IOServiceThread& operator=(const IOServiceThread&) = delete;

	private:
		IOThreadID								thread_id_;
		IOServiceThreadManager&					manager_;
		asio::io_service						io_service_;
		std::unique_ptr<std::thread>			thread_;
		std::unique_ptr<asio::io_service::work>	work_;
		TCPSessionQueue							session_queue_;
	};
}

#endif
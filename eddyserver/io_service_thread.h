#ifndef __IO_SERVICE_THREAD_H__
#define __IO_SERVICE_THREAD_H__

#include <thread>
#include <asio/io_service.hpp>
#include "tcp_session_queue.h"

namespace eddy
{
	class IOServiceThreadManager;

	class IOServiceThread : public std::enable_shared_from_this < IOServiceThread >
	{
		friend class IOServiceThreadManager;
		typedef std::unique_ptr<asio::io_service::work> WorkPointer;

	public:
		IOServiceThread(ThreadID id, IOServiceThreadManager &manager);
		~IOServiceThread();

	public:
		void run_thread();

		void join();

		void stop();

		template <typename CompletionHandler>
		void post(ASIO_MOVE_ARG(CompletionHandler) handler)
		{
			io_service_.post(handler);
		}

	public:
		ThreadID id() const
		{
			return thread_id_;
		}

		asio::io_service& io_service()
		{
			return io_service_;
		}

		IOServiceThreadManager& manager()
		{
			return manager_;
		}

		TCPSessionQueue& session_queue()
		{
			return session_queue_;
		}

	private:
		void run();

	private:
		IOServiceThread(const IOServiceThread&) = delete;
		IOServiceThread& operator=(const IOServiceThread&) = delete;

	private:
		ThreadID						thread_id_;
		IOServiceThreadManager&			manager_;
		asio::io_service				io_service_;
		std::unique_ptr<std::thread>	thread_;
		WorkPointer						work_;
		TCPSessionQueue					session_queue_;
	};
}

#endif
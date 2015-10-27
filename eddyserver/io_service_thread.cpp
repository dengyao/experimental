#include "io_service_thread.h"

#include <iostream>


namespace eddy
{
	IOServiceThread::IOServiceThread(IOServiceThreadManager &manager)
		: work_(nullptr)
		, manager_(manager)
	{

	}

	IOServiceThread::~IOServiceThread()
	{

	}

	void IOServiceThread::run()
	{
		if (work_ == nullptr)
		{
			work_.reset(new asio::io_service::work(io_service_));
		}

		asio::error_code error;
		io_service_.run(error);

		if (error)
		{
			std::cerr << error.message() << std::endl;
		}
	}

	void IOServiceThread::run_thread()
	{
		if (thread_ == nullptr)
		{
			thread_ = std::make_shared<std::thread>(std::bind(&IOServiceThread::run, this));
		}
	}

	size_t IOServiceThread::load() const
	{
		return session_queue_.size();
	}

	void IOServiceThread::join()
	{
		if (thread_ != nullptr)
		{
			thread_->join();
		}
	}

	void IOServiceThread::stop()
	{
		if (work_ != nullptr)
		{
			work_.reset();
		}
	}
}
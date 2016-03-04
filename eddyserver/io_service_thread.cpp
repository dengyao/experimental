#include "io_service_thread.h"
#include <iostream>

namespace eddy
{
	IOServiceThread::IOServiceThread(ThreadID id, IOServiceThreadManager &manager)
		: work_(nullptr)
		, thread_id_(id)
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
			thread_.reset(new std::thread(std::bind(&IOServiceThread::run, this)));
		}
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
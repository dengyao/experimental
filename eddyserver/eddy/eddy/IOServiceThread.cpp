#include "IOServiceThread.h"
#include <iostream>
#include "TCPSession.h"

namespace eddy
{
	IOServiceThread::IOServiceThread(IOThreadID id, IOServiceThreadManager &manager)
		: timeout_(0)
		, work_(nullptr)
		, thread_id_(id)
		, manager_(manager)
		, timer_(io_service_)
	{

	}

	IOServiceThread::~IOServiceThread()
	{

	}

	void IOServiceThread::Run()
	{
		if (work_ == nullptr)
		{
			work_.reset(new asio::io_service::work(io_service_));
		}

		timer_.expires_from_now(std::chrono::seconds(1));
		timer_.async_wait(std::bind(&IOServiceThread::CheckTimeOut, shared_from_this(), std::placeholders::_1));

		asio::error_code error;
		io_service_.run(error);

		if (error)
		{
			std::cerr << error.message() << std::endl;
		}
	}

	void IOServiceThread::RunThread()
	{
		if (thread_ == nullptr)
		{
			thread_.reset(new std::thread(std::bind(&IOServiceThread::Run, this)));
		}
	}

	void IOServiceThread::Join()
	{
		if (thread_ != nullptr)
		{
			thread_->join();
		}
	}

	void IOServiceThread::Stop()
	{
		if (work_ != nullptr)
		{
			work_.reset();
		}
	}

	void IOServiceThread::SetSessionTimeout(uint64_t timeout)
	{
		timeout_ = std::chrono::seconds(timeout);
	}

	void IOServiceThread::CheckTimeOut(asio::error_code error)
	{
		if (!error)
		{
			if (timeout_ > std::chrono::seconds(0))
			{
				auto now_time = std::chrono::steady_clock::now();
				session_queue_.Foreach([&](const SessionPointer &session)->void
				{
					auto interval = now_time - session->LastActivityTime();
					if (interval >= timeout_)
					{
						session->Close();
					}		
				});
			}

			timer_.expires_from_now(std::chrono::seconds(1));
			timer_.async_wait(std::bind(&IOServiceThread::CheckTimeOut, shared_from_this(), std::placeholders::_1));
		}
		else
		{
			std::cerr << error.message() << std::endl;
		}
	}
}
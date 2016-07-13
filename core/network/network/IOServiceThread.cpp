#include "IOServiceThread.h"
#include <iostream>
#include "TCPSession.h"

namespace network
{
	IOServiceThread::IOServiceThread(IOThreadID id, IOServiceThreadManager &manager)
		: timeout_(0)
		, thread_id_(id)
		, manager_(manager)
		, timer_(io_service_)
	{
	}

	void IOServiceThread::Run()
	{
		if (work_ == nullptr)
		{		
			work_ = std::make_unique<asio::io_service::work>(io_service_);
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
			thread_ = std::make_unique<std::thread>(std::bind(&IOServiceThread::Run, this));
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

	void IOServiceThread::SetSessionTimeout(uint64_t seconds)
	{
		timeout_ = std::chrono::seconds(seconds);
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
					auto interval = now_time - session->LastActivity();
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
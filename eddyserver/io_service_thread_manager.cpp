#include "io_service_thread_manager.h"
#include <limits>
#include <cassert>
#include "io_service_thread.h"
#include "tcp_session_handle.h"

namespace eddy
{
	const size_t kMainThreadIndex = 0;

	IOServiceThreadManager::IOServiceThreadManager(size_t thread_num)
	{
		if (thread_num <= kMainThreadIndex)
		{
			std::abort();
		}

		threads_.resize(thread_num);
		for (size_t i = 0; i < threads_.size(); ++i)
		{
			threads_[i] = std::make_shared<IOServiceThread>(*this);
		}
	}

	IOServiceThreadManager::~IOServiceThreadManager()
	{
		stop();
	}

	void IOServiceThreadManager::run()
	{
		if (threads_.empty())
		{
			return;
		}

		for (size_t i = 0; i < threads_.size(); ++i)
		{
			if (i != kMainThreadIndex)
			{
				threads_[i]->run_thread();
			}
		}
		threads_[kMainThreadIndex]->run();
	}

	void IOServiceThreadManager::stop()
	{
		if (threads_.empty())
		{
			return;
		}

		for (size_t i = 0; i < threads_.size(); ++i)
		{
			if (i != kMainThreadIndex)
			{
				threads_[i]->stop();
			}
		}

		for (size_t i = 0; i < threads_.size(); ++i)
		{
			if (i != kMainThreadIndex)
			{
				threads_[i]->join();
			}
		}

		threads_[kMainThreadIndex]->stop();
	}

	ThreadPointer IOServiceThreadManager::thread()
	{
		if (threads_.size() == 1)
		{
			return threads_[kMainThreadIndex];
		}

		size_t min_element_index = kMainThreadIndex;
		size_t min_element_value = std::numeric_limits<size_t>::max();
		for (size_t i = 0; i < threads_.size(); ++i)
		{
			if (kMainThreadIndex != i && threads_[i]->load() < min_element_value)
			{
				min_element_index = i;
			}
		}
		return threads_[min_element_index];
	}

	ThreadPointer IOServiceThreadManager::thread(ThreadID id)
	{
		for (size_t i = 0; i < threads_.size(); ++i)
		{
			if (threads_[i]->id() == id)
			{
				return threads_[i];
			}
		}
		return nullptr;
	}

	ThreadPointer IOServiceThreadManager::main_thread()
	{
		return threads_[kMainThreadIndex];
	}

	void IOServiceThreadManager::on_session_connect(SessionPointer session_ptr, SessionHandlerPointer handler_ptr)
	{
		TCPSessionID id = IDGenerator::kInvalidID;
		if (id_generator_.get(id))
		{
			handler_ptr->init(id, session_ptr->thread()->id(), this);
			session_handler_map_.insert(std::make_pair(id, handler_ptr));
			session_ptr->thread()->post(std::bind(&TCPSession::init, session_ptr, id));
			handler_ptr->on_connect();
		}
	}

	void IOServiceThreadManager::on_session_close(TCPSessionID id)
	{
		assert(IDGenerator::kInvalidID != id);
		SessionHandlerMap::iterator itr = session_handler_map_.find(id);
		if (itr != session_handler_map_.end())
		{
			SessionHandlerPointer handler_ptr = itr->second;
			if (handler_ptr != nullptr)
			{
				handler_ptr->on_close();
				handler_ptr->dispose();
			}
			session_handler_map_.erase(itr);
		}

		if (IDGenerator::kInvalidID != id)
		{
			id_generator_.put(id);
		}
	}

	SessionHandlerPointer IOServiceThreadManager::session_handler(TCPSessionID id) const
	{
		SessionHandlerMap::const_iterator itr = session_handler_map_.find(id);
		if (itr != session_handler_map_.end())
		{
			return itr->second;
		}
		return SessionHandlerPointer();
	}
}
#include "io_service_thread_manager.h"

#include <limits>
#include <cassert>
#include "io_service_thread.h"
#include "tcp_session_handle.h"

const size_t kMainThreadIndex = 0;

IOServiceThreadManager::IOServiceThreadManager(size_t thread_num)
{
	if (thread_num > kMainThreadIndex)
	{
		std::abort();
	}

	threads_.reserve(thread_num);
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

IOServiceThread& IOServiceThreadManager::thread()
{
	if (threads_.size() == 1)
	{
		return *threads_[kMainThreadIndex];
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
	return *threads_[min_element_index];
}

bool IOServiceThreadManager::thread(ThreadID id, IOServiceThread *&ret_thread)
{
	ret_thread = nullptr;
	for (size_t i = 0; i < threads_.size(); ++i)
	{
		if (threads_[i]->id() == id)
		{
			ret_thread = threads_[i].get();
			return true;
		}
	}
	return false;
}

IOServiceThread& IOServiceThreadManager::main_thread()
{
	return *threads_[kMainThreadIndex];
}

void IOServiceThreadManager::on_session_connect(SessionPointer session, SessionHandlerPointer handler)
{
	TCPSessionID id = IDGenerator::kInvalidID;
	if (id_generator_.get(id))
	{
		handler->init(id, session->thread().id(), this);
		session_handler_map_.insert(std::make_pair(id, handler));
		session->thread().post(std::bind(&TCPSession::init, session, id));
		handler->on_connect();
	}
}

void IOServiceThreadManager::on_session_close(TCPSessionID id)
{
	assert(IDGenerator::kInvalidID != id);
	SessionHandlerMap::iterator itr = session_handler_map_.find(id);
	if (itr != session_handler_map_.end())
	{
		SessionHandlerPointer handler = itr->second;
		if (handler != nullptr)
		{
			handler->on_close();
			handler->dispose();
		}
		session_handler_map_.erase(itr);
	}

	if (IDGenerator::kInvalidID != id)
	{
		id_generator_.put(id);
	}
}

IOServiceThreadManager::SessionHandlerPointer IOServiceThreadManager::session_handler(TCPSessionID id) const
{
	SessionHandlerMap::const_iterator itr = session_handler_map_.find(id);
	if (itr != session_handler_map_.end())
	{
		return itr->second;
	}
	return SessionHandlerPointer();
}
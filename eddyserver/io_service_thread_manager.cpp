#include "io_service_thread_manager.h"

#include <queue>
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

	typedef std::pair<size_t, size_t> value_type;
	std::priority_queue<value_type, std::vector<value_type>, std::greater<value_type>> que;
	for (size_t i = 0; i < threads_.size(); ++i)
	{
		que.push(std::make_pair(threads_[i]->load(), i));
	}
	return *threads_[que.top().second];
}

IOServiceThread& IOServiceThreadManager::thread(ThreadID id)
{
	for (size_t i = 0; i < threads_.size(); ++i)
	{
		if (threads_[i]->id() == id)
		{
			return *threads_[i];
		}
	}
	throw std::runtime_error("not found thread");
}

IOServiceThread& IOServiceThreadManager::main_thread()
{
	return *threads_[kMainThreadIndex];
}

void IOServiceThreadManager::on_session_connect(SessionPointer session, SessionHandlerPointer handler)
{
	TCPSessionID id;
	if (id_generator_.get(id))
	{
		handler->init(id, session->thread().id(), this);
		session_handler_map_.insert(std::make_pair(id, handler));
		session->thread().post(std::bind(&tcp_session::init, session, id));
		handler->on_connect();
	}
}

void IOServiceThreadManager::on_session_close(session_id id)
{
	assert(id != kInvalidSessionID);
	session_handler_map::iterator itr = session_handler_map_.find(id);
	if (itr != session_handler_map_.end())
	{
		session_handler_ptr handler = itr->second;
		if (handler != nullptr)
		{
			handler->on_close();
			handler->dispose();
		}
		session_handler_map_.erase(itr);
	}
	id_generator_.put(id);
}

io_service_thread_manager::session_handler_ptr IOServiceThreadManager::session_handler(session_id id) const
{
	session_handler_map::const_iterator itr = session_handler_map_.find(id);
	if (itr != session_handler_map_.end())
	{
		return itr->second;
	}
	return session_handler_ptr();
}
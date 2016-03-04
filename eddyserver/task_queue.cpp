#include "task_queue.h"
#include <chrono>
#include <cassert>
#include <climits>
#include <numeric>


namespace eddy
{
	TaskThread::TaskThread()
		: finished_(false)
	{
		thread_.reset(new std::thread(std::bind(&TaskThread::run, this)));
	}

	TaskThread::~TaskThread()
	{

	}

	void TaskThread::join()
	{
		termminiate();
		thread_->join();
	}

	void TaskThread::termminiate()
	{
		finished_ = true;
		condition_incoming_task_.notify_one();
	}

	size_t TaskThread::load() const
	{
		size_t count = 0;
		{
			std::lock_guard<std::mutex> lock(list_task_mutex_);
			count = list_task_.size();
		}
		return count;
	}

	void TaskThread::wait_for_idle()
	{
		while (load() > 0)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(200));
		}
	}

	size_t TaskThread::append(const Task &task)
	{
		size_t count = 0;
		if (!finished_)
		{
			{
				std::lock_guard<std::mutex> locker(list_task_mutex_);
				list_task_.push_back(task);
				count = list_task_.size();
			}

			if (count == 1)
			{
				condition_incoming_task_.notify_one();
			}
		}
		return count;
	}

	void TaskThread::run()
	{
		while (!finished_)
		{
			Task task;
			{
				std::lock_guard<std::mutex> locker(list_task_mutex_);
				if (!list_task_.empty())
				{
					task = std::move(list_task_.front());
					list_task_.pop_front();
				}
			}

			if (task != nullptr)
			{
				task();
			}

			while (load() == 0)
			{
				std::unique_lock<std::mutex> locker(condition_mutex_);
				if (locker.mutex())
				{
					condition_incoming_task_.wait(locker);
				}
			}
		}
	}

	/************************************************************************/

	TaskQueue::TaskQueue(size_t thread_num)
	{
		assert(thread_num > 0);
		if (thread_num == 0)
		{
			std::abort();
		}
		threads_.resize(thread_num);
	}

	TaskQueue::~TaskQueue()
	{

	}

	void TaskQueue::join()
	{
		for (auto &item : threads_)
		{
			item.join();
		}
	}

	void TaskQueue::termminiate()
	{
		for (auto &item : threads_)
		{
			item.termminiate();
		}
	}

	void TaskQueue::wait_for_idle()
	{
		while (true)
		{
			size_t sum = std::accumulate(threads_.begin(), threads_.end(), 0, [](size_t sum, const TaskThread &item)
			{
				sum += item.load();
			});

			if (sum > 0)
			{
				std::this_thread::sleep_for(std::chrono::microseconds(200));
			}
			else
			{
				break;
			}
		}
	}

	size_t TaskQueue::append(const TaskThread::Task &task)
	{
		size_t min_index = 0;
		size_t min_value = threads_[min_index].load();
		for (size_t index = 0; index < threads_.size(); ++index)
		{
			size_t value = threads_[index].load();
			if (value <= min_value)
			{
				min_value = value;
				min_index = index;
			}
		}
		threads_[min_index].append(task);
	}
}
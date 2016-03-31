#include "TaskQueue.h"
#include <chrono>
#include <cassert>
#include <climits>
#include <numeric>


namespace eddy
{
	TaskThread::TaskThread()
		: finished_(false)
	{
		thread_.reset(new std::thread(std::bind(&TaskThread::Run, this)));
	}

	TaskThread::~TaskThread()
	{

	}

	void TaskThread::Join()
	{
		Termminiate();
		thread_->join();
	}

	void TaskThread::Termminiate()
	{
		finished_ = true;
		condition_incoming_task_.notify_one();
	}

	size_t TaskThread::Load() const
	{
		size_t count = 0;
		{
			std::lock_guard<std::mutex> lock(list_task_mutex_);
			count = list_task_.size();
		}
		return count;
	}

	void TaskThread::WaitForIdle()
	{
		while (Load() > 0)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(200));
		}
	}

	size_t TaskThread::Append(const Task &task)
	{
		size_t count = 0;
		if (!finished_)
		{
			{
				std::lock_guard<std::mutex> lock(list_task_mutex_);
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

	void TaskThread::Run()
	{
		while (!finished_)
		{
			Task task;
			{
				std::lock_guard<std::mutex> lock(list_task_mutex_);
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

			while (Load() == 0)
			{
				std::unique_lock<std::mutex> lock(condition_mutex_);
				if (lock.mutex())
				{
					condition_incoming_task_.wait(lock);
				}
			}
		}
	}

	/************************************************************************/

	TaskQueue::TaskQueue(size_t thread_num)
	{
		assert(thread_num > 0);
		threads_.resize(thread_num);
		for (size_t i = 0; i < threads_.size(); ++i)
		{
			threads_[i].reset(new TaskThread);
		}
	}

	TaskQueue::~TaskQueue()
	{

	}

	void TaskQueue::Join()
	{
		for (auto &item : threads_)
		{
			item->Join();
		}
	}

	void TaskQueue::Termminiate()
	{
		for (auto &item : threads_)
		{
			item->Termminiate();
		}
	}

	void TaskQueue::WaitForIdle()
	{
		while (true)
		{
			size_t task_sum = std::accumulate(threads_.begin(), threads_.end(), 0, [=](size_t sum, const TaskThreadPointer &item)
			{
				return sum += item->Load();
			});

			if (task_sum > 0)
			{
				std::this_thread::sleep_for(std::chrono::microseconds(200));
			}
			else
			{
				break;
			}
		}
	}

	void TaskQueue::Append(const TaskThread::Task &task)
	{
		if (!threads_.empty())
		{
			size_t min_index = 0;
			size_t min_value = threads_[min_index]->Load();
			for (size_t index = 0; index < threads_.size(); ++index)
			{
				size_t value = threads_[index]->Load();
				if (value <= min_value)
				{
					min_value = value;
					min_index = index;
				}
			}
			threads_[min_index]->Append(task);
		}
		else
		{
			task();
		}
	}
}
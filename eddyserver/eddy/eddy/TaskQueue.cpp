#include "TaskQueue.h"
#include <chrono>
#include <cassert>
#include <climits>
#include <numeric>

namespace eddy
{
	TaskQueue::TaskQueue()
		: finished_(false)
	{
		thread_ = std::make_unique<std::thread>(std::bind(&TaskQueue::Run, this));
	}

	void TaskQueue::Join()
	{
		Termminiate();
		thread_->join();
	}

	void TaskQueue::Termminiate()
	{
		finished_ = true;
		not_empty_.notify_one();
	}

	size_t TaskQueue::Load() const
	{
		size_t count = 0;
		{
			std::lock_guard<std::mutex> lock(queue_task_mutex_);
			count = queue_task_.size();
		}
		return count;
	}

	void TaskQueue::WaitForIdle()
	{
		while (Load() > 0)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(200));
		}
	}

	size_t TaskQueue::Append(const Task &task)
	{
		assert(task != nullptr);
		size_t count = 0;
		if (!finished_)
		{
			{
				std::lock_guard<std::mutex> lock(queue_task_mutex_);
				if (task != nullptr)
				{
					queue_task_.push_back(task);

				}
				count = queue_task_.size();
			}

			if (count == 1)
			{
				not_empty_.notify_one();
			}
		}
		return count;
	}

	void TaskQueue::Run()
	{
		while (!finished_)
		{
			Task task;
			{
				std::lock_guard<std::mutex> lock(queue_task_mutex_);
				if (!queue_task_.empty())
				{
					task = std::move(queue_task_.front());
					queue_task_.pop_front();
				}
			}
			task();

			while (Load() == 0)
			{
				std::unique_lock<std::mutex> lock(not_empty_mutex_);
				if (lock.mutex())
				{
					not_empty_.wait(lock);
				}
			}
		}
	}

	TaskQueuePool::TaskQueuePool(size_t thread_num)
	{
		assert(thread_num > 0);
		threads_.resize(thread_num);
		for (size_t i = 0; i < threads_.size(); ++i)
		{
			threads_[i] = std::make_unique<TaskQueue>();
		}
	}

	void TaskQueuePool::Join()
	{
		for (auto &item : threads_)
		{
			item->Join();
		}
	}

	void TaskQueuePool::Termminiate()
	{
		for (auto &item : threads_)
		{
			item->Termminiate();
		}
	}

	void TaskQueuePool::WaitForIdle()
	{
		while (true)
		{
			size_t task_sum = std::accumulate(threads_.begin(), threads_.end(), 0, [=](size_t sum, const TaskQueuePointer &item)
			{
				return sum += item->Load();
			});

			if (task_sum == 0)
			{
				break;
			}
			std::this_thread::sleep_for(std::chrono::microseconds(200));
		}
	}

	void TaskQueuePool::Append(const TaskQueue::Task &task)
	{
		assert(task != nullptr);
		if (task == nullptr)
		{
			return;
		}

		if (threads_.empty())
		{
			task();
		}
		else
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
	}
}
#include "TaskQueue.h"
#include <chrono>
#include <cassert>
#include <iostream>


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
	if (!finished_)
	{
		finished_ = true;
		condition_incoming_task_.notify_one();
	}
}

size_t TaskQueue::Load() const
{
	size_t load = 0;
	{
		std::lock_guard<std::mutex> lock(queue_task_mutex_);
		load = queue_task_.size();
	}
	return load;
}

void TaskQueue::WaitForIdle()
{
	while (Load() > 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

size_t TaskQueue::Append(Task &task)
{
	size_t size = 0;
	if (!finished_)
	{
		{
			std::lock_guard<std::mutex> lock(queue_task_mutex_);
			queue_task_.push_back(task);
			size = queue_task_.size();
		}

		if (size == 1)
		{
			condition_incoming_task_.notify_one();
		}
	}
	return size;
}

void TaskQueue::Run()
{
	while (!finished_)
	{
		while (!finished_ && Load() == 0)
		{
			std::unique_lock<std::mutex> lock(condition_mutex_);
			if (lock.mutex())
			{
				condition_incoming_task_.wait_for(lock, std::chrono::milliseconds(100));
			}
		}

		Task current_task;
		{
			std::lock_guard<std::mutex> lock(queue_task_mutex_);
			if (!queue_task_.empty())
			{
				current_task = std::move(queue_task_.front());
			}
		}
		if (current_task != nullptr)
		{
			try
			{
				current_task();
			}
			catch (const std::exception &e)
			{
				std::cerr << e.what() << std::endl;
			}		
			std::lock_guard<std::mutex> lock(queue_task_mutex_);
			queue_task_.pop_front();
		}
	}
}
#include "TaskPools.h"
#include <cassert>
#include <algorithm>


TaskPools::TaskPools(size_t thread_num)
{
	assert(thread_num > 0);
	for (size_t i = 0; i < thread_num; ++i)
	{
		threads_.push_back(std::make_unique<TaskQueue>());
	}
}

size_t TaskPools::Count() const
{
	return threads_.size();
}

void TaskPools::Join()
{
	for (TaskQueuePointer &task_queue : threads_)
	{
		task_queue->Join();
	}
}

void TaskPools::Termminiate()
{
	for (TaskQueuePointer &task_queue : threads_)
	{
		task_queue->Termminiate();
	}
}

size_t TaskPools::Load(size_t index) const
{
	return index < threads_.size() ? threads_[index]->Load() : 0;
}

void TaskPools::WaitForIdle()
{
	for (TaskQueuePointer &task_queue : threads_)
	{
		task_queue->WaitForIdle();
	}
}

void TaskPools::Append(TaskQueue::Task &task)
{
	assert(!threads_.empty());
	if (!threads_.empty())
	{
		std::vector<TaskQueuePointer>::iterator min_load_thread;
		{
			std::lock_guard<std::mutex> lock(mutex_);
			min_load_thread = std::min_element(threads_.begin(), threads_.end(), [](const TaskQueuePointer &a, const TaskQueuePointer &b)->bool
			{
				return a->Load() < b->Load();
			});
		}
		(*min_load_thread)->Append(task);
	}
}
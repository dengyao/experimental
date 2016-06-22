#ifndef __TASK_POOLS_H__
#define __TASK_POOLS_H__

#include <vector>
#include "TaskQueue.h"

class TaskPools
{
public:
	typedef std::unique_ptr<TaskQueue> TaskQueuePointer;

public:
	TaskPools(size_t thread_num);

public:
	size_t Count() const;

	void Join();

	void Termminiate();

	size_t Load(size_t index) const;

	void WaitForIdle();

	void Append(TaskQueue::Task &task);

private:
	TaskPools(const TaskPools&) = delete;
	TaskPools& operator= (const TaskPools&) = delete;

private:
	mutable std::mutex            mutex_;
	std::vector<TaskQueuePointer> threads_;
};

#endif

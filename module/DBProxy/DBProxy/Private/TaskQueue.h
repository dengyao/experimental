#ifndef __TASK_QUEUE_H__
#define __TASK_QUEUE_H__

#include <list>
#include <mutex>
#include <atomic>
#include <thread>
#include <memory>
#include <functional>
#include <condition_variable>

class TaskQueue
{
public:
	typedef std::function<void()> Task;
	typedef std::unique_ptr<std::thread> ThreadPointer;

public:
	TaskQueue();

public:
	void Join();

	void Termminiate();

	size_t Load() const;

	void WaitForIdle();

	size_t Append(Task &task);

private:
	void Run();

private:
	TaskQueue(const TaskQueue&) = delete;
	TaskQueue& operator= (const TaskQueue&) = delete;

private:
	std::atomic_bool		finished_;
	ThreadPointer           thread_;
	std::list<Task>         queue_task_;
	mutable std::mutex      queue_task_mutex_;
	mutable std::mutex      condition_mutex_;
	std::condition_variable condition_incoming_task_;
};

#endif
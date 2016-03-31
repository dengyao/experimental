#ifndef __TASK_QUEUE_H__
#define __TASK_QUEUE_H__

#include <deque>
#include <vector>
#include <mutex>
#include <atomic>
#include <memory>
#include <thread>
#include <functional>
#include <condition_variable>

namespace eddy
{
	class TaskQueue final
	{
	public:
		typedef std::function<void()> Task;

	public:
		TaskQueue();
		~TaskQueue() = default;

	public:
		void Join();

		void Termminiate();

		size_t Load() const;

		void WaitForIdle();

		size_t Append(const Task &task);

	protected:
		TaskQueue(const TaskQueue&) = delete;
		TaskQueue& operator= (const TaskQueue&) = delete;

	private:
		void Run();

	private:
		std::atomic<bool>				finished_;
		std::unique_ptr<std::thread>	thread_;
		std::deque<Task>				queue_task_;
		mutable std::mutex				queue_task_mutex_;
		std::condition_variable			not_empty_;
		mutable std::mutex				not_empty_mutex_;
	};

	class TaskQueuePool final
	{
		typedef std::unique_ptr<TaskQueue> TaskQueuePointer;

	public:
		TaskQueuePool(size_t thread_num);
		~TaskQueuePool() = default;

	public:
		void Join();

		void Termminiate();

		void WaitForIdle();

		void Append(const TaskQueue::Task &task);

	protected:
		TaskQueuePool(const TaskQueuePool&) = delete;
		TaskQueuePool& operator= (const TaskQueuePool&) = delete;

	private:
		std::vector<TaskQueuePointer>	threads_;
	};
}

#endif

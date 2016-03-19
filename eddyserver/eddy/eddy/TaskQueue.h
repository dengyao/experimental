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
	class TaskThread final
	{
	public:
		typedef std::function<void()> Task;

	public:
		TaskThread();
		~TaskThread();

	public:
		void Join();

		void Termminiate();

		size_t Load() const;

		void WaitForIdle();

		size_t Append(const Task &task);

	protected:
		TaskThread(const TaskThread&) = delete;
		TaskThread& operator= (const TaskThread&) = delete;

	private:
		void Run();

	private:
		std::atomic<bool>				finished_;
		std::unique_ptr<std::thread>	thread_;
		std::deque<Task>				list_task_;
		mutable std::mutex				list_task_mutex_;
		mutable std::mutex				condition_mutex_;
		std::condition_variable			condition_incoming_task_;
	};

	class TaskQueue final
	{
		typedef std::unique_ptr<TaskThread> TaskThreadPointer;

	public:
		TaskQueue(size_t thread_num);
		~TaskQueue();

	public:
		void Join();

		void Termminiate();

		void WaitForIdle();

		void Append(const TaskThread::Task &task);

	protected:
		TaskQueue(const TaskQueue&) = delete;
		TaskQueue& operator= (const TaskQueue&) = delete;

	private:
		std::vector<TaskThreadPointer>	threads_;
	};
}

#endif

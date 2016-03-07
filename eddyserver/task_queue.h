#ifndef __TASK_QUEUE_H__
#define __TASK_QUEUE_H__

#include <vector>
#include <deque>
#include <mutex>
#include <atomic>
#include <memory>
#include <thread>
#include <functional>

namespace eddy
{
	class TaskThread
	{
	public:
		typedef std::function<void()> Task;

	public:
		TaskThread();
		~TaskThread();

	public:
		void join();

		void termminiate();

		size_t load() const;

		void wait_for_idle();

		size_t append(const Task &task);

	protected:
		TaskThread(const TaskThread &) = delete;
		TaskThread& operator= (const TaskThread &) = delete;

	private:
		void run();

	private:
		std::atomic<bool>				finished_;
		std::unique_ptr<std::thread>	thread_;
		std::deque<Task>				list_task_;
		mutable std::mutex				list_task_mutex_;
		mutable std::mutex				condition_mutex_;
		std::condition_variable			condition_incoming_task_;
	};

	class TaskQueue
	{
	public:
		TaskQueue(size_t thread_num);
		~TaskQueue();

	public:
		void join();

		void termminiate();

		void wait_for_idle();

		void append(const TaskThread::Task &task);

		void stop();

	protected:
		TaskQueue(const TaskQueue &) = delete;
		TaskQueue& operator= (const TaskQueue &) = delete;

	private:
		std::vector<TaskThread> threads_;
	};
}

#endif

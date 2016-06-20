#ifndef __PROXY_MANAGER_H__
#define __PROXY_MANAGER_H__

#include <vector>
#include <queue>
#include <functional>
#include <unordered_map>
#include "Types.h"
#include "Connector.h"
#include "TaskQueue.h"

namespace dbproxy
{
	const size_t kMaxTaskQueueSize = 1024;

	template <typename Database>
	class ProxyManager
	{
		struct STaskTarget
		{
			const int number;
			const std::string db;
			const std::string command;
			STaskTarget(int n, const char *d, const char *c) : number(n) , db(d), command(c)
			{
			}

			void Execute() const
			{

			}
		};

		struct STaskResult
		{
			const int number;
		};

	public:
		typedef std::queue<std::queue<STaskTarget>> TaskQueue;
		typedef std::unique_ptr<Connector<Database>> ConnectorPointer;
		typedef std::unordered_multimap<std::string, ConnectorPointer> ConnectorMap;

	public:
		ProxyManager(std::vector<ConnectorPointer> &&connectors, const std::shared_ptr<TaskQueue> &pools)
			: pools_(pools)
		{
			for (size_t i = 0; i < connectors.size(); ++i)
			{
				assert(connectors[i] != nullptr);
				free_connectors_.insert(std::make_pair(connectors[i]->Name(), connectors[i]))
			}
		}

		void Append(int number, const char *db, const char *command)
		{

		}

		void GetCompletionQueue(TaskQueue &queue)
		{
			std::swap()
		}

	private:
		void OnCompletionTask(const ErrorCode &code, DatabaseResult<MySQL> &result)
		{

		}

	private:
		TaskQueue task_queue_;
		TaskQueue completion_queue_;
		ConnectorMap used_connectors_;
		ConnectorMap free_connectors_;
		std::shared_ptr<TaskQueue> pools_;
	};
}

#endif

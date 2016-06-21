#ifndef __PROXY_MANAGER_H__
#define __PROXY_MANAGER_H__

#include <vector>
#include <list>
#include <functional>
#include <unordered_map>
#include "Types.h"
#include "Connector.h"
#include "TaskQueue.h"

namespace dbproxy
{
	template <typename Database>
	class ProxyManager
	{
	public:
		typedef std::unique_ptr<Connector<Database>> ConnectorPointer;
		typedef std::function<void(int, ConnectorPointer&&, DatabaseResult<Database>&, const ErrorCode&)> CompleteNotify;

		template <typename Value>
		class SafeQueue
		{
		public:
			bool Take(Value &value)
			{
				std::lock_guard<std::mutex> lock(mutex_);
				if (container_.empty())
				{
					return false;
				}
				value = std::move(container_.front());
				container_.pop_front();
				return true;
			}

			void Append(const Value &value)
			{
				std::lock_guard<std::mutex> lock(mutex_);
				container_.push_back(value);
			}

			void Append(Value &&value)
			{
				std::lock_guard<std::mutex> lock(mutex_);
				container_.push_back(value);
			}

			size_t Size() const
			{
				std::lock_guard<std::mutex> lock(mutex_);
				return container_.size();
			}

		private:
			SafeQueue(const SafeQueue&) = delete;
			SafeQueue& operator= (const SafeQueue&) = delete;

		private:
			std::mutex mutex_;
			std::list<Value> container_;
		};

		template <typename Key, typename Value>
		class SafeMultimap
		{
		public:
			bool Take(const Key &key, Value &value)
			{
				std::lock_guard<std::mutex> lock(mutex_);
				std::unordered_multimap<Key, Value>::iterator range = container_.equal_range(key);
				if (range.frist == range.second)
				{
					return false;
				}
				value = std::move(range.first->second);
				container_.erase(range.first);
				return true;
			}

			void Append(const Key &key, const Value &value)
			{
				std::lock_guard<std::mutex> lock(mutex_);
				container_.insert(std::make_pair(key, value));
			}

			void Append(const Key &key, Value &&value)
			{
				std::lock_guard<std::mutex> lock(mutex_);
				container_.insert(std::make_pair(key, value));
			}

			size_t Size() const
			{
				std::lock_guard<std::mutex> lock(mutex_);
				return container_.size();
			}

		private:
			SafeMultimap(const SafeMultimap&) = delete;
			SafeMultimap& operator= (const SafeMultimap&) = delete;

		private:
			std::mutex mutex_;
			std::unordered_multimap<Key, Value> container_;
		};

		class Actor : public std::enable_shared_from_this<Actor>
		{
		public:
			Actor(int number, CommandType type, const char *command, const size_t length, ConnectorPointer &&connector, const CompleteNotify &callback)
				: type_(type)
				, number_(number)
				, connector_(connector)
				, complete_notify_(callback)
			{
				command_.resize(length);
				memcpy(command_.data(), command, length);
				assert(connector_ != nullptr);
				assert(complete_notify_ != nullptr);
			}

			void Implement()
			{
				ErrorCode error_code;
				DatabaseResult<Database> result;
				switch (type_)
				{
				case dbproxy::CommandType::kSelect:
					result = connector_->Select(command_, error_code);
					break;
				case dbproxy::CommandType::kInsert:
					result = connector_->Insert(command_, error_code);
					break;
				case dbproxy::CommandType::kUpdate:
					result = connector_->Update(command_, error_code);
					break;
				case dbproxy::CommandType::kDelete:
					result = connector_->Delete(command_, error_code);
					break;
				default:
					assert(false);
					break;
				}
				complete_notify_(number_, std::move(connector_), result, error_code);
			}

		private:
			Actor(const Actor&) = delete;
			Actor& operator= (const Actor&) = delete;

		private:
			const int					number_;
			const CommandType			type_;
			std::vector<char>			command_;
			ConnectorPointer			connector_;
			const CompleteNotify&		complete_notify_;
		};
		typedef std::shared_ptr<Actor> ActorPointer;

	public:
		ProxyManager(std::vector<ConnectorPointer> &&connectors, std::shared_ptr<TaskQueue> &pools, unsigned int bocklog)
			: pools_(pools)
			, bocklog_(bocklog)
			, complete_notify_(std::bind(&ProxyManager::OnCompletionTask, this))
		{
			for (size_t i = 0; i < connectors.size(); ++i)
			{
				assert(connectors[i] != nullptr);
				free_connectors_.Append(connectors[i]->Name(), std::move(connectors[i]));
			}
		}

		bool Append(int number, CommandType type, const char *db, const char *command, const size_t length)
		{
			if (waiting_queue_.Size() >= bocklog_)
			{
				return false;
			}

			ConnectorPointer connector;
			if (free_connectors_.Take(db, connector))
			{
				ActorPointer actor = std::make_shared<Actor>(number, type, command, length, std::move(connector), complete_notify_);
				ongoing_queue_.Append(number, actor);
				pools_->Append(std::bind(&Actor::Implement, actor));
			}
			else
			{
				ActorPointer actor = std::make_shared<Actor>(number, type, command, length, std::move(connector), complete_notify_);
				waiting_queue_.Append(number, std::move(actor));
			}
			return true;
		}

		void GetCompletionQueue(TaskQueue &queue)
		{
			
		}

	private:
		void OnCompletionTask(int number, ConnectorPointer &&connector, DatabaseResult<Database> &result, const ErrorCode &code)
		{
			free_connectors_.Append(connector->Name(), connector);
		}

	private:
		const unsigned int							bocklog_;
		std::shared_ptr<TaskQueue>					pools_;
		SafeQueue<ActorPointer>						waiting_queue_;
		SafeMultimap<int, ActorPointer>				ongoing_queue_;
		SafeMultimap<std::string, ConnectorPointer>	free_connectors_;
		const CompleteNotify						complete_notify_;
	};
}

#endif

#ifndef __PROXY_MANAGER_H__
#define __PROXY_MANAGER_H__

#include <set>
#include <vector>
#include "Types.h"
#include "Connector.h"
#include "TaskPools.h"
#include "ContainerSafe.h"

namespace dbproxy
{
	template <typename Database>
	class ProxyManager
	{
	public:
		class Result
		{
		public:
			Result(int number, ErrorCode &&code, DatabaseResult<Database> &&result)
				: number_(number)
				, code_(std::forward<ErrorCode>(code))
				, result_(std::forward<DatabaseResult<Database>>(result))
			{
			}

			Result(Result &&other)
			{
				number_ = other.number_;
				code_ = std::move(other.code_);
				result_ = std::move(other.result_);
				other.number_ = 0;
			}

			int GetNumber() const
			{
				return number_;
			}

			ErrorCode& GetErrorCode()
			{
				return code_;
			}

			const ErrorCode& GetErrorCode() const
			{
				return code_;
			}

			DatabaseResult<Database>& GetResult()
			{
				return result_;
			}

			const DatabaseResult<Database>& GetResult() const
			{
				return result_;
			}

		private:
			Result(const Result&) = delete;
			Result& operator= (const Result&) = delete;

		private:
			int number_;
			ErrorCode code_;
			DatabaseResult<Database> result_;
		};

		typedef std::unique_ptr<Connector<Database>> ConnectorPointer;

	private:
		class Actor;
		typedef std::shared_ptr<Actor> ActorPointer;
		typedef std::function<void(ActorPointer &actor, ErrorCode&&, DatabaseResult<Database>&&)> CompleteNotify;

		class Actor : public std::enable_shared_from_this<Actor>
		{
		public:
			Actor(int number, CommandType type, const char *command, const size_t length, ConnectorPointer &&connector, const CompleteNotify &callback)
				: type_(type)
				, number_(number)
				, complete_notify_(callback)
				, connector_(std::forward<ConnectorPointer>(connector))
			{
				command_.resize(length);
				memcpy(command_.data(), command, length);
				assert(complete_notify_ != nullptr);
			}

			int GetNumber() const
			{
				return number_;
			}

			ConnectorPointer& GetConnector()
			{
				return connector_;
			}

			void SetConnector(ConnectorPointer &&connector)
			{
				assert(connector_ == nullptr && connector != nullptr);
				connector_ = std::move(connector);
			}

			void Processing()
			{
				ErrorCode error_code;
				DatabaseResult<Database> result;
				assert(connector_ != nullptr);
				if (connector_ != nullptr)
				{
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
				}
				ActorPointer actor = Actor::shared_from_this();
				complete_notify_(actor, std::move(error_code), std::move(result));
			}

		private:
			Actor(const Actor&) = delete;
			Actor& operator= (const Actor&) = delete;

		private:
			const int number_;
			const CommandType type_;
			std::vector<char> command_;
			ConnectorPointer connector_;
			const CompleteNotify& complete_notify_;
		};

	public:
		ProxyManager(std::vector<ConnectorPointer> &&connectors, std::shared_ptr<TaskPools> &pools, unsigned int bocklog)
			: pools_(pools)
			, bocklog_(bocklog)
			, complete_notify_(std::bind(&ProxyManager::OnCompletionTask, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
		{
			for (size_t i = 0; i < connectors.size(); ++i)
			{
				assert(connectors[i] != nullptr);
				waiting_queue_.Append(connectors[i]->Name());
				free_connectors_.Append(connectors[i]->Name(), std::move(connectors[i]));		
			}
		}

		size_t GetCompletionQueue(std::vector<Result> &completion)
		{
			return completion_queue_.TakeAll(completion);
		}

		bool Append(int number, CommandType type, const char *db, const char *command, const size_t length)
		{
			QueueSafe<ActorPointer> *waiting = nullptr;
			if (!waiting_queue_.Get(db, waiting))
			{
				return false;
			}

			if (waiting->Size() >= bocklog_)
			{
				return false;
			}

			if (ongoing_queue_.IsExist(number))
			{
				throw std::logic_error("number cannot be repeated");
			}

			ConnectorPointer connector;
			if (free_connectors_.Take(db, connector))
			{
				ActorPointer actor = std::make_shared<Actor>(number, type, command, length, std::move(connector), complete_notify_);
				TaskQueue::Task task = std::bind(&Actor::Processing, actor);
				ongoing_queue_.Append(number, actor);
				pools_->Append(task);
			}
			else
			{
				ActorPointer actor = std::make_shared<Actor>(number, type, command, length, std::move(connector), complete_notify_);
				waiting->Append(std::move(actor));
			}
			return true;
		}

	private:
		void OnCompletionTask(ActorPointer &actor, ErrorCode &&code, DatabaseResult<Database> &&result)
		{
			ConnectorPointer connector(std::move(actor->GetConnector()));
			completion_queue_.Append(Result(actor->GetNumber(), std::forward<ErrorCode>(code), std::forward<DatabaseResult<Database>>(result)));
			if (ongoing_queue_.Take(actor->GetNumber(), actor))
			{
				QueueSafe<ActorPointer> *waiting = nullptr;
				if (waiting_queue_.Get(connector->Name(), waiting))
				{			
					if (waiting->Take(actor))
					{
						actor->SetConnector(std::move(connector));
						TaskQueue::Task task = std::bind(&Actor::Processing, actor);
						ongoing_queue_.Append(actor->GetNumber(), actor);
						pools_->Append(task);
					}
					else
					{
						free_connectors_.Append(connector->Name(), std::move(connector));
					}
				}
				else
				{
					free_connectors_.Append(connector->Name(), std::move(connector));
				}
			}
			else
			{
				assert(false);
				free_connectors_.Append(connector->Name(), std::move(connector));
			}
		}

	private:
		const unsigned int                            bocklog_;
		std::shared_ptr<TaskPools>                    pools_;
		MultimapSafe<std::string, ConnectorPointer>   free_connectors_;
		MapSafe<std::string, QueueSafe<ActorPointer>> waiting_queue_;
		MapSafe<int, ActorPointer>                    ongoing_queue_;
		QueueSafe<Result>                             completion_queue_;
		const CompleteNotify                          complete_notify_;
	};
}

#endif

#ifndef __PROXY_MANAGER_H__
#define __PROXY_MANAGER_H__

#include <set>
#include <vector>
#include "base.h"
#include "Types.h"
#include "Connector.h"
#include "ContainerSafe.h"

namespace dbproxy
{
	class Result
	{
	public:
		Result(uint32_t identifier, ErrorCode &&code, ByteArray &&result)
			: identifier_(identifier)
			, error_code_(std::forward<ErrorCode>(error_code_))
			, result_(std::forward<ByteArray>(result))
		{
		}

		Result(Result &&other)
		{
			identifier_ = other.identifier_;
			error_code_ = std::move(other.error_code_);
			result_ = std::move(other.result_);
			other.identifier_ = 0;
		}

		int GetIdentifier() const
		{
			return identifier_;
		}

		ErrorCode& GetErrorCode()
		{
			return error_code_;
		}

		const ErrorCode& GetErrorCode() const
		{
			return error_code_;
		}

		ByteArray& GetResult()
		{
			return result_;
		}

		const ByteArray& GetResult() const
		{
			return result_;
		}

	private:
		Result(const Result&) = delete;
		Result& operator= (const Result&) = delete;

	private:
		uint32_t  identifier_;
		ErrorCode error_code_;
		ByteArray result_;
	};

	template <typename Database>
	class ProxyManager
	{
		class Actor;
		typedef std::shared_ptr<Actor> ActorPointer;
		typedef std::function<void(ActorPointer &actor, ErrorCode&&, DatabaseResult<Database>&&)> CompleteNotify;

	public:
		typedef std::unique_ptr<Connector<Database>> ConnectorPointer;

	public:
		ProxyManager(std::vector<ConnectorPointer> &&connectors, TaskPools &pools, unsigned int bocklog)
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

		size_t GetCompletionQueue(std::vector<Result> &lists)
		{
			return completion_queue_.TakeAll(lists);
		}

		void Append(uint32_t identifier, ActionType type, const char *db, const char *command, const size_t length)
		{
			QueueSafe<ActorPointer> *waiting = nullptr;
			if (!waiting_queue_.Get(db, waiting))
			{
				throw NotFoundDatabase();
			}

			if (waiting->Size() >= bocklog_)
			{
				throw ResourceInsufficiency();
			}

#ifdef _DEBUG
			if (ongoing_queue_.IsExist(identifier))
			{
				assert(false);
				return;
			}
#endif // _DEBUG
			
			ConnectorPointer connector;
			if (free_connectors_.Take(db, connector))
			{
				ActorPointer actor = std::make_shared<Actor>(identifier, type, command, length, std::move(connector), complete_notify_);
				TaskQueue::Task task = std::bind(&Actor::Processing, actor);
				ongoing_queue_.Append(identifier, actor);
				pools_.Append(task);
			}
			else
			{
				ActorPointer actor = std::make_shared<Actor>(identifier, type, command, length, std::move(connector), complete_notify_);
				waiting->Append(std::move(actor));
			}
		}

	private:
		void OnCompletionTask(ActorPointer &actor, ErrorCode &&code, DatabaseResult<Database> &&result)
		{
			ConnectorPointer connector(std::move(actor->GetConnector()));
			completion_queue_.Append(Result(actor->GetIdentifier(), std::forward<ErrorCode>(code), std::move(result.GetData())));
			if (ongoing_queue_.Take(actor->GetIdentifier(), actor))
			{
				QueueSafe<ActorPointer> *waiting = nullptr;
				if (waiting_queue_.Get(connector->Name(), waiting))
				{			
					if (waiting->Take(actor))
					{
						actor->SetConnector(std::move(connector));
						TaskQueue::Task task = std::bind(&Actor::Processing, actor);
						ongoing_queue_.Append(actor->GetIdentifier(), actor);
						pools_.Append(task);
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
		class Actor : public std::enable_shared_from_this<Actor>
		{
		public:
			Actor(uint32_t identifier, ActionType type, const char *command, const size_t length, ConnectorPointer &&connector, const CompleteNotify &callback)
				: type_(type)
				, identifier_(identifier)
				, complete_notify_(callback)
				, connector_(std::forward<ConnectorPointer>(connector))
			{
				command_.resize(length);
				memcpy(command_.data(), command, length);
				assert(complete_notify_ != nullptr);
			}

			int GetIdentifier() const
			{
				return identifier_;
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
					case dbproxy::ActionType::kSelect:
						result = connector_->Select(command_, error_code);
						break;
					case dbproxy::ActionType::kInsert:
						result = connector_->Insert(command_, error_code);
						break;
					case dbproxy::ActionType::kUpdate:
						result = connector_->Update(command_, error_code);
						break;
					case dbproxy::ActionType::kDelete:
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
			const ActionType      type_;
			const uint32_t        identifier_;
			ByteArray             command_;
			ConnectorPointer      connector_;
			const CompleteNotify& complete_notify_;
		};

	private:
		const unsigned int                            bocklog_;
		TaskPools&                                    pools_;
		MultimapSafe<std::string, ConnectorPointer>   free_connectors_;
		MapSafe<std::string, QueueSafe<ActorPointer>> waiting_queue_;
		MapSafe<uint32_t, ActorPointer>               ongoing_queue_;
		QueueSafe<Result>                             completion_queue_;
		const CompleteNotify                          complete_notify_;
	};
}

#endif

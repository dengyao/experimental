#include "AgentImpl.h"
#include <cassert>
#include <cstring>
#include <common/TaskPools.h>

/************************************************************************/

HandleResult::HandleResult(uint32_t sequence, ErrorCode &&code, ByteArray &&result)
	: sequence_(sequence)
	, error_code_(std::forward<ErrorCode>(code))
	, result_(std::forward<ByteArray>(result))
{
}

HandleResult::HandleResult(HandleResult &&other)
{
	sequence_ = other.sequence_;
	error_code_ = std::move(other.error_code_);
	result_ = std::move(other.result_);
	other.sequence_ = 0;
}

// 获取序列号
int HandleResult::GetSequence() const
{
	return sequence_;
}

// 获取错误代码
ErrorCode& HandleResult::GetErrorCode()
{
	return error_code_;
}

const ErrorCode& HandleResult::GetErrorCode() const
{
	return error_code_;
}

// 获取处理结果
ByteArray& HandleResult::GetHandleResult()
{
	return result_;
}

const ByteArray& HandleResult::GetHandleResult() const
{
	return result_;
}

/************************************************************************/
/************************************************************************/

class Actor : public std::enable_shared_from_this<Actor>
{
	Actor(const Actor&) = delete;
	Actor& operator= (const Actor&) = delete;

public:
	Actor(uint32_t sequence, ActionType type, const char *command,
		const size_t length, ConnectorPointer &&connector, const AgentImpl::CompleteCallback &callback)
		: type_(type)
		, sequence_(sequence)
		, complete_callback_(callback)
		, connector_(std::forward<ConnectorPointer>(connector))
	{
		command_.resize(length);
		memcpy(command_.data(), command, length);
		assert(complete_callback_ != nullptr);
	}

	int GetSequence() const
	{
		return sequence_;
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
		ByteArray bytes;
		ErrorCode error_code;
		assert(connector_ != nullptr);
		if (connector_ != nullptr)
		{
			switch (type_)
			{
			case ActionType::kCall:
				bytes = connector_->Call(command_, error_code);
				break;
			case ActionType::kSelect:
				bytes = connector_->Select(command_, error_code);
				break;
			case ActionType::kInsert:
				bytes = connector_->Insert(command_, error_code);
				break;
			case ActionType::kUpdate:
				bytes = connector_->Update(command_, error_code);
				break;
			case ActionType::kDelete:
				bytes = connector_->Delete(command_, error_code);
				break;
			default:
				assert(false);
				break;
			}
		}
		ActorPointer actor = Actor::shared_from_this();
		complete_callback_(actor, std::move(error_code), std::move(bytes));
	}

private:
	const ActionType					type_;
	const uint32_t						sequence_;
	ByteArray							command_;
	ConnectorPointer					connector_;
	const AgentImpl::CompleteCallback&	complete_callback_;
};

/************************************************************************/
/************************************************************************/

AgentImpl::AgentImpl(std::vector<ConnectorPointer> &&connectors, TaskPools &pools, unsigned int backlog)
	: pools_(pools)
	, bocklog_(backlog)
	, complete_callback_(std::bind(&AgentImpl::OnCompletionTask, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
{
	for (size_t i = 0; i < connectors.size(); ++i)
	{
		assert(connectors[i] != nullptr);
		waiting_queue_.Append(connectors[i]->Name());
		free_connectors_.Append(connectors[i]->Name(), std::move(connectors[i]));
	}
}

// 添加任务
void AgentImpl::AppendTask(uint32_t sequence, ActionType type, const char *db, const char *command, const size_t length)
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
	if (ongoing_queue_.IsExist(sequence))
	{
		assert(false);
		return;
	}
#endif // _DEBUG

	ConnectorPointer connector;
	if (free_connectors_.Take(db, connector))
	{
		ActorPointer actor = std::make_shared<Actor>(sequence, type, command, length, std::move(connector), complete_callback_);
		TaskQueue::Task task = std::bind(&Actor::Processing, actor);
		ongoing_queue_.Append(sequence, actor);
		pools_.Append(task);
	}
	else
	{
		ActorPointer actor = std::make_shared<Actor>(sequence, type, command, length, std::move(connector), complete_callback_);
		waiting->Append(std::move(actor));
	}
}

// 获取已完成任务
size_t AgentImpl::GetCompletedTask(std::vector<HandleResult> &tasks)
{
	return completed_queue_.TakeAll(tasks);
}

// 处理完成回调
void AgentImpl::OnCompletionTask(ActorPointer &actor, ErrorCode &&code, ByteArray &&result)
{
	ConnectorPointer connector(std::move(actor->GetConnector()));
	completed_queue_.Append(HandleResult(actor->GetSequence(), std::forward<ErrorCode>(code), std::forward<ByteArray>(result)));
	if (ongoing_queue_.Take(actor->GetSequence(), actor))
	{
		QueueSafe<ActorPointer> *waiting = nullptr;
		if (waiting_queue_.Get(connector->Name(), waiting))
		{
			if (waiting->Take(actor))
			{
				actor->SetConnector(std::move(connector));
				TaskQueue::Task task = std::bind(&Actor::Processing, actor);
				ongoing_queue_.Append(actor->GetSequence(), actor);
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
#ifndef __AGENT_MYSQL_H__
#define __AGENT_MYSQL_H__

#include "ContainerSafe.h"
#include "InterfaceConnector.h"

class Actor;
class TaskPools;
typedef std::shared_ptr<Actor> ActorPointer;

class HandleResult
{
	HandleResult(const HandleResult&) = delete;
	HandleResult& operator= (const HandleResult&) = delete;

public:
	HandleResult(uint32_t sequence, ErrorCode &&code, ByteArray &&bytes);
	HandleResult(HandleResult &&other);

public:
	// 获取序列号
	int GetSequence() const;

	// 获取错误代码
	ErrorCode& GetErrorCode();
	const ErrorCode& GetErrorCode() const;

	// 获取处理结果
	ByteArray& GetHandleResult();
	const ByteArray& GetHandleResult() const;

private:
	ByteArray	result_;
	uint32_t	sequence_;
	ErrorCode	error_code_;
};

/************************************************************************/

class AgentMySQL
{
public:
	typedef std::function<void(ActorPointer &actor, ErrorCode&&, ByteArray&&)> CompleteCallback;

public:
	AgentMySQL(std::vector<ConnectorPointer> &&connectors, TaskPools &pools, unsigned int backlog);

public:
	// 获取已完成任务
	size_t GetCompletedTask(std::vector<HandleResult> &tasks);

	// 添加任务
	void AppendTask(uint32_t sequence, ActionType type, const std::string &db, std::string &&command);

private:
	void OnCompletionTask(ActorPointer &actor, ErrorCode &&code, ByteArray &&result);

private:
	const unsigned int								bocklog_;
	TaskPools&										pools_;
	MultimapSafe<std::string, ConnectorPointer>		free_connectors_;
	MapSafe<std::string, QueueSafe<ActorPointer> >	waiting_queue_;
	MapSafe<uint32_t, ActorPointer>					ongoing_queue_;
	QueueSafe<HandleResult>							completed_queue_;
	const CompleteCallback							complete_callback_;
};

#endif

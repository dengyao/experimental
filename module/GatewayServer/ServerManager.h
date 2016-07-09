#ifndef __SERVER_MANAGER_H__
#define __SERVER_MANAGER_H__

#define USE_DICT_STRUCTURE 1

#include <map>
#include <vector>
#include <memory>

namespace google
{
	namespace protobuf
	{
		class Message;
	}
}

class SessionHandlePoint;

struct ChildNode
{
	int child_id;
	bool working;
	std::vector<SessionHandlePoint*> session_lists;
};

#if (USE_DICT_STRUCTURE)
struct ServerNode
{
	int node_type;
	std::vector<ChildNode> child_lists;
};

typedef std::vector<ServerNode> ServerNodeContainer;
#else
struct ServerNode
{
	int node_type;
	std::map<int, ChildNode> child_lists;
};

typedef std::map<int, ServerNode> ServerNodeContainer;
#endif

class ServerManager
{
	struct FindChild
	{
		int node_type;
		int child_id;
	};

public:
	ServerManager();

	~ServerManager();

private:
	// 服务器登录
	void OnServerLogin(sads asdsa, google::protobuf::Message *message);

	// 服务器暂停服务
	void OnServerPauseWork(int node_type, int child_id, google::protobuf::Message *message);

	// 服务器继续服务器
	void OnServerContinueWor(int node_type, int child_id, google::protobuf::Message *message);

	// 转发服务器消息
	void OnForwardServerMessage(int src_node_type, int src_child_id, int dst_node_type, int dst_child_id, google::protobuf::Message *message);

	// 广播服务器消息
	void OnBroadcastServerMessage(int src_node_type, int src_child_id, const std::vector<FindChild> &dst_lists, google::protobuf::Message *message);

private:
	ServerNodeContainer node_lists_;
};

#endif

#ifndef __SERVER_MANAGER_H__
#define __SERVER_MANAGER_H__

#include <vector>
#include <memory>
#include <unordered_map>
#include <eddyserver.h>

namespace google
{
	namespace protobuf
	{
		class Message;
	}
}

class ServerManager
{
	// 节点索引(快速查找)
	struct NodeIndex
	{
		int node_type;
		int child_id;

		NodeIndex()
			: node_type(0) , child_id(0)
		{
		}
	};

	// 子节点信息
	struct ChildNode
	{
		int child_id;
		bool working;
		std::vector<eddy::TCPSessionID> session_lists;

		ChildNode()
			: child_id(0), working(false)
		{
		}
	};

	// 服务器节点信息
	struct ServerNode
	{
		int node_type;
		std::unordered_map<int, ChildNode> child_lists;

		ServerNode()
			: node_type(0)
		{
		}
	};

public:
	ServerManager(eddy::IOServiceThreadManager &threads);

	~ServerManager();

private:
	// 服务器登录
	void OnServerLogin(eddy::TCPSessionHandler &session, google::protobuf::Message *message);

	// 服务器暂停服务
	void OnServerPauseWork(eddy::TCPSessionHandler &session, int node_type, int child_id, google::protobuf::Message *message);

	// 服务器继续服务器
	void OnServerContinueWor(eddy::TCPSessionHandler &session, int node_type, int child_id, google::protobuf::Message *message);

	// 转发服务器消息
	void OnForwardServerMessage(eddy::TCPSessionHandler &session, int dst_node_type, int dst_child_id, google::protobuf::Message *message);

	// 广播服务器消息
	void OnBroadcastServerMessage(eddy::TCPSessionHandler &session, const std::vector<NodeIndex> &dst_lists, google::protobuf::Message *message);

private:
	// 查找服务器节点
	bool FindServerNodeBySessionID(eddy::TCPSessionID session_id, ChildNode *&out_child_node);

private:
	eddy::IOServiceThreadManager& threads_;
	std::unordered_map<int, ServerNode> server_lists_;
	std::unordered_map<eddy::TCPSessionID, NodeIndex> node_index_;
};

#endif

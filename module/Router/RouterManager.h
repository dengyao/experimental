#ifndef __ROUTER_MANAGER_H__
#define __ROUTER_MANAGER_H__

#include <network.h>

namespace google
{
	namespace protobuf
	{
		class Message;
	}
}

class RouterManager
{
	// 节点索引(快速查找)
	struct NodeIndex
	{
		int node_type;
		int child_id;

		NodeIndex()
			: node_type(0), child_id(0)
		{
		}
	};

	// 子节点信息
	struct ChildNode
	{
		int node_type;
		int child_id;
		std::vector<network::TCPSessionID> session_lists;

		ChildNode()
			: node_type(0), child_id(0)
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

	typedef std::function<void(network::TCPSessionHandler &session, google::protobuf::Message *message, network::NetMessage &added)> MessageHandler;

public:
	RouterManager(network::IOServiceThreadManager &threads);
	~RouterManager();

public:
	// 处理消息
	void HandleMessage(network::TCPSessionHandler &session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 处理服务器下线
	void HandleServerOffline(network::TCPSessionHandler &session);

private:
	// 服务器登录
	void OnServerLogin(network::TCPSessionHandler &session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 转发服务器消息
	void OnForwardServerMessage(network::TCPSessionHandler &session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 广播服务器消息
	void OnBroadcastServerMessage(network::TCPSessionHandler &session, google::protobuf::Message *message, network::NetMessage &buffer);

public:
	// 回复错误码
	void RespondErrorCode(network::TCPSessionHandler &session, int error_code, const char *what = nullptr);

private:
	// 查找服务器节点
	bool FindServerNode(int node_type, int child_id, ChildNode *&out_child_node);
	bool FindServerNodeBySessionID(network::TCPSessionID session_id, ChildNode *&out_child_node);

	// 查找服务器节点会话
	bool FindServerNodeSession(int node_type, int child_id, network::SessionHandlePointer &out_session);

private:
	network::IOServiceThreadManager&                     threads_;
	std::unordered_map<int, ServerNode>               server_lists_;
	std::unordered_map<network::TCPSessionID, NodeIndex> node_index_;
};

#endif

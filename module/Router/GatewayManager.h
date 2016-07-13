#ifndef __SERVER_MANAGER_H__
#define __SERVER_MANAGER_H__

#include <eddyserver.h>

namespace google
{
	namespace protobuf
	{
		class Message;
	}
}

class GatewayManager
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
		std::vector<eddy::TCPSessionID> session_lists;

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

	typedef std::function<void(eddy::TCPSessionHandler &session, google::protobuf::Message *message, eddy::NetMessage &added)> MessageHandler;

public:
	GatewayManager(eddy::IOServiceThreadManager &threads);
	~GatewayManager();

public:
	// 处理消息
	void HandleMessage(eddy::TCPSessionHandler &session, google::protobuf::Message *message, eddy::NetMessage &buffer);

	// 处理服务器下线
	void HandleServerOffline(eddy::TCPSessionHandler &session);

private:
	// 服务器登录
	void OnServerLogin(eddy::TCPSessionHandler &session, google::protobuf::Message *message, eddy::NetMessage &buffer);

	// 转发服务器消息
	void OnForwardServerMessage(eddy::TCPSessionHandler &session, google::protobuf::Message *message, eddy::NetMessage &buffer);

	// 广播服务器消息
	void OnBroadcastServerMessage(eddy::TCPSessionHandler &session, google::protobuf::Message *message, eddy::NetMessage &buffer);

public:
	// 回复错误码
	void RespondErrorCode(eddy::TCPSessionHandler &session, int error_code, const char *what = nullptr);

private:
	// 查找服务器节点
	bool FindServerNode(int node_type, int child_id, ChildNode *&out_child_node);
	bool FindServerNodeBySessionID(eddy::TCPSessionID session_id, ChildNode *&out_child_node);

	// 查找服务器节点会话
	bool FindServerNodeSession(int node_type, int child_id, eddy::SessionHandlePointer &out_session);

private:
	eddy::IOServiceThreadManager&                     threads_;
	std::unordered_map<int, ServerNode>               server_lists_;
	std::unordered_map<eddy::TCPSessionID, NodeIndex> node_index_;
};

#endif

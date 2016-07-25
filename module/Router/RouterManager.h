#ifndef __ROUTER_MANAGER_H__
#define __ROUTER_MANAGER_H__

#include <network.h>
#include <ProtobufDispatcher.h>

namespace google
{
	namespace protobuf
	{
		class Message;
	}
}

class SessionHandle;

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

public:
	RouterManager(network::IOServiceThreadManager &threads);
	~RouterManager();

public:
	// 接收消息
	bool OnMessage(SessionHandle *session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 连接关闭
	void OnClose(SessionHandle *session);

	// 发送错误码
	void SendErrorCode(SessionHandle *session, network::NetMessage &buffer, int error_code, const char *what = nullptr);

private:
	// 未定义消息
	bool OnUnknownMessage(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 服务器登录
	bool OnServerLogin(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 查询路由信息
	bool OnQueryRouterInfo(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 转发服务器消息
	bool OnForwardServerMessage(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer);

	// 广播服务器消息
	bool OnBroadcastServerMessage(network::TCPSessionHandler *ession, google::protobuf::Message *message, network::NetMessage &buffer);

private:
	// 更新统计数据
	void UpdateStatisicalData(asio::error_code error_code);

	// 查找服务器节点
	bool FindServerNode(int node_type, int child_id, ChildNode *&out_child_node);
	bool FindServerNodeBySessionID(network::TCPSessionID session_id, ChildNode *&out_child_node);

	// 查找服务器节点会话
	bool FindServerNodeSession(int node_type, int child_id, network::SessionHandlePointer &out_session);

private:
	network::IOServiceThreadManager&						threads_;
	asio::steady_timer										timer_;
	const std::function<void(asio::error_code)>				wait_handler_;
	std::unordered_map<int, ServerNode>						server_lists_;
	std::unordered_map<network::TCPSessionID, NodeIndex>	node_index_;
	ProtobufDispatcher										dispatcher_;
};

#endif

#ifndef __ROUTER_CLIENT_H__
#define __ROUTER_CLIENT_H__

#include <set>
#include <network.h>

namespace google
{
	namespace protobuf
	{
		class Message;
	}
}

class RouterClientHandle;
class AsyncReconnectHandle;

class RouterClient
{
	friend class RouterClientHandle;
	friend class AsyncReconnectHandle;

public:
	struct Context
	{
		int node_type;
		int child_id;
		Context()
			: node_type(0), child_id(0)
		{
		}
	};

	typedef std::function<void(const Context &context, google::protobuf::Message*, network::NetMessage &buffer)> Callback;

public:
	RouterClient(network::IOServiceThreadManager &threads, asio::ip::tcp::endpoint &endpoint, size_t connection_num, int node_type, int child_id = 1);
	~RouterClient();

public:
	// 服务器节点类型
	int GetNodeType() const;

	// 服务器子节点id
	int GetChildNodeID() const;

	// 设置消息回调
	void SetMessageCallback(const Callback &cb);

	// 响应消息
	void Respond(google::protobuf::Message *message);

	// 发送消息
	void Send(int node_type, int child_id, google::protobuf::Message *message);

private:
	// 连接事件
	void OnConnected(RouterClientHandle *client);

	// 断开连接事件
	void OnDisconnect(RouterClientHandle *client);

	// 接受消息事件
	void OnMessage(RouterClientHandle *client, google::protobuf::Message *message);

private:
	// 清理所有连接
	void Clear();

	// 初始化连接
	void InitConnections();

	// 异步重连
	void AsyncReconnect();

	// 异步重连结果
	void AsyncReconnectResult(AsyncReconnectHandle &handler, asio::error_code error_code);

	// 更新计时器
	void UpdateTimer(asio::error_code error_code);

	// 创建会话处理器
	network::SessionHandlePointer CreateSessionHandle();

private:
	const int                                   node_type_;
	const int                                   child_id_;
	Callback                                    message_cb_;
	Context                                     context_;
	const size_t                                connection_num_;
	unsigned short                              connecting_num_;
	network::IOServiceThreadManager&            threads_;
	std::set<std::shared_ptr<bool> >            lifetimes_;
	network::TCPClient					        client_creator_;
	std::vector<RouterClientHandle*>            client_lists_;
	asio::ip::tcp::endpoint                     endpoint_;
	asio::steady_timer                          timer_;
	const std::function<void(asio::error_code)> wait_handler_;
	size_t                                      next_client_index_;
};

#endif

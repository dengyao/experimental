﻿#ifndef __ROUTER_CLIENT_H__
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

class ConnectRouterFailed : public std::runtime_error
{
public:
	ConnectRouterFailed(const char *message)
		: std::runtime_error(message)
	{
	}
};

class RouterSessionHandle;
class AsyncReconnectHandle;

class RouterClient
{
	friend class RouterSessionHandle;
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

	typedef std::function<void(RouterClient *client, google::protobuf::Message*, network::NetMessage &buffer)> Callback;

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

	// 回复消息
	void Reply(google::protobuf::Message *message);

	// 发送消息
	void Send(int dst_node_type, int dst_child_id, google::protobuf::Message *message);

	// 广播消息
	void Broadcast(const std::vector<int> &dst_type_lists,  google::protobuf::Message *message);

private:
	// 连接事件
	void OnConnected(RouterSessionHandle *session);

	// 断开连接事件
	void OnDisconnect(RouterSessionHandle *session);

	// 接受消息事件
	void OnMessage(RouterSessionHandle *session, google::protobuf::Message *message, network::NetMessage &buffer);

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
	void OnUpdateTimer(asio::error_code error_code);

	// 获取会话处理器
	RouterSessionHandle* GetRouterSessionHandle();

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
	network::TCPClient					        session_handle_creator_;
	std::vector<RouterSessionHandle*>           session_handle_lists_;
	asio::ip::tcp::endpoint                     endpoint_;
	asio::steady_timer                          timer_;
	const std::function<void(asio::error_code)> wait_handler_;
	size_t                                      next_client_index_;
};

#endif
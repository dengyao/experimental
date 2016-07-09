#include "ServerManager.h"
#include <algorithm>
#include <proto/MessageHelper.h>
#include <proto/internal.protocol.pb.h>


ServerManager::ServerManager(eddy::IOServiceThreadManager &threads)
	: threads_(threads)
{

}

ServerManager::~ServerManager()
{
}

// 查找服务器节点
bool ServerManager::FindServerNodeBySessionID(eddy::TCPSessionID session_id, ChildNode *&out_child_node)
{
	out_child_node = nullptr;
	auto found = node_index_.find(session_id);
	if (found != node_index_.end())
	{
		return false;
	}

	auto node_found = server_lists_.find(found->second.node_type);
	if (node_found == server_lists_.end())
	{
		return false;
	}

	auto child_found = node_found->second.child_lists.find(found->second.child_id);
	if (child_found == node_found->second.child_lists.end())
	{
		return false;
	}

	auto session_found = std::find(child_found->second.session_lists.begin(), child_found->second.session_lists.end(), session_id);
	if (session_found == child_found->second.session_lists.end())
	{
		return false;
	}

	out_child_node = &child_found->second;
	return true;
}

// 服务器登录
void ServerManager::OnServerLogin(eddy::TCPSessionHandler &session, google::protobuf::Message *message)
{
	auto request = dynamic_cast<internal::LoginGatewayReq*>(message);
	assert(request != nullptr);
	if (request == nullptr)
	{
		return;
	}

	// 是否重复登录
	auto found = node_index_.find(session.SessionID());
	if (found != node_index_.end())
	{
		return;
	}

	// 服务器节点登录
	auto node_found = server_lists_.find(request->type());
	if (node_found == server_lists_.end())
	{
		ServerNode &node_lists = server_lists_[request->type()];
		node_lists.node_type = request->type();

		ChildNode &child_node_lists = node_lists.child_lists[request->child_id()];
		child_node_lists.working = true;
		child_node_lists.child_id = request->child_id();
		child_node_lists.session_lists.push_back(session.SessionID());
	}
	else
	{
		auto child_found = node_found->second.child_lists.find(request->child_id());
		if (child_found == node_found->second.child_lists.end())
		{
			ChildNode &child_node_lists = node_found->second.child_lists[request->child_id()];
			child_node_lists.working = true;
			child_node_lists.child_id = request->child_id();
			child_node_lists.session_lists.push_back(session.SessionID());
		}
		else
		{
			child_found->second.session_lists.push_back(session.SessionID());
		}
	}

	// 新建节点索引
	NodeIndex index;
	index.node_type = request->type();
	index.child_id = request->child_id();
	node_index_.insert(std::make_pair(session.SessionID(), index));
}

// 服务器暂停服务
void ServerManager::OnServerPauseWork(eddy::TCPSessionHandler &session, int node_type, int child_id, google::protobuf::Message *message)
{
	auto request = dynamic_cast<internal::PauseWorkReq*>(message);
	assert(request != nullptr);
	if (request == nullptr)
	{
		return;
	}

	ChildNode *child_node = nullptr;
	if (!FindServerNodeBySessionID(session.SessionID(), child_node))
	{
		// 服务器子节点未登录
	}
	else
	{
		child_node->working = false;
	}
}

// 服务器继续服务器
void ServerManager::OnServerContinueWor(eddy::TCPSessionHandler &session, int node_type, int child_id, google::protobuf::Message *message)
{
	auto request = dynamic_cast<internal::ContinueWorkReq*>(message);
	assert(request != nullptr);
	if (request == nullptr)
	{
		return;
	}

	ChildNode *child_node = nullptr;
	if (!FindServerNodeBySessionID(session.SessionID(), child_node))
	{
		// 服务器子节点未登录
	}
	else
	{
		child_node->working = true;
	}
}

// 转发服务器消息
void ServerManager::OnForwardServerMessage(eddy::TCPSessionHandler &session, int dst_node_type, int dst_child_id, google::protobuf::Message *message)
{
	ChildNode *child_node = nullptr;
	if (!FindServerNodeBySessionID(session.SessionID(), child_node))
	{
		// 服务器子节点未登录
	}
	else
	{
		child_node->working = true;
	}
}

// 广播服务器消息
void ServerManager::OnBroadcastServerMessage(eddy::TCPSessionHandler &session, const std::vector<NodeIndex> &dst_lists, google::protobuf::Message *message)
{
	ChildNode *child_node = nullptr;
	if (!FindServerNodeBySessionID(session.SessionID(), child_node))
	{
		// 服务器子节点未登录
	}
	else
	{
		child_node->working = true;
	}
}
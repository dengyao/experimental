#include "RouterManager.h"
#include <proto/MessageHelper.h>
#include <proto/internal.protocol.pb.h>

RouterManager::RouterManager(network::IOServiceThreadManager &threads)
	: threads_(threads)
{
}

RouterManager::~RouterManager()
{
}

// 回复错误码
void RouterManager::RespondErrorCode(network::TCPSessionHandler &session, int error_code, const char *what)
{
	internal::RouterErrorRsp error;
	error.set_error_code(static_cast<internal::ErrorCode>(error_code));
	if (what != nullptr)
	{
		error.set_what(what);
	}
	network::NetMessage message;
	PackageMessage(&error, message);
	session.Send(message);
}

// 查找服务器节点
bool RouterManager::FindServerNode(int node_type, int child_id, ChildNode *&out_child_node)
{
	out_child_node = nullptr;
	auto node_found = server_lists_.find(node_type);
	if (node_found == server_lists_.end())
	{
		return false;
	}

	auto child_found = node_found->second.child_lists.find(child_id);
	if (child_found == node_found->second.child_lists.end())
	{
		return false;
	}
	out_child_node = &child_found->second;
	return true;
}

bool RouterManager::FindServerNodeBySessionID(network::TCPSessionID session_id, ChildNode *&out_child_node)
{
	auto found = node_index_.find(session_id);
	if (found == node_index_.end())
	{
		return false;
	}
	return FindServerNode(found->second.node_type, found->second.child_id, out_child_node);
}

// 查找节点会话
bool RouterManager::FindServerNodeSession(int node_type, int child_id, network::SessionHandlePointer &out_session)
{
	auto node_found = server_lists_.find(node_type);
	if (node_found == server_lists_.end())
	{
		return false;
	}

	auto child_found = node_found->second.child_lists.find(child_id);
	if (child_found == node_found->second.child_lists.end())
	{
		return false;
	}

	if (child_found->second.session_lists.empty())
	{
		return false;
	}

	network::TCPSessionID session_id = 0;
	if (child_found->second.session_lists.size() == 1)
	{
		session_id = child_found->second.session_lists.front();
	}
	else
	{
		session_id = child_found->second.session_lists[rand() % child_found->second.session_lists.size()];
	}
	out_session = threads_.SessionHandler(session_id);
	return out_session != nullptr;
}

// 处理收到消息
void RouterManager::HandleMessage(network::TCPSessionHandler &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	if (dynamic_cast<internal::LoginRouterReq*>(message) != nullptr)
	{
		OnServerLogin(session, message, buffer);
	}
	else if (dynamic_cast<internal::ForwardMessageReq*>(message) != nullptr)
	{
		OnForwardServerMessage(session, message, buffer);
	}
	else if (dynamic_cast<internal::BroadcastMessageReq*>(message) != nullptr)
	{
		OnBroadcastServerMessage(session, message, buffer);
	}
	else
	{
		RespondErrorCode(session, internal::kInvalidProtocol, message->GetTypeName().c_str());
	}
}

// 处理服务器下线
void RouterManager::HandleServerOffline(network::TCPSessionHandler &session)
{
	auto found = node_index_.find(session.SessionID());
	if (found == node_index_.end())
	{
		return;
	}

	auto node_found = server_lists_.find(found->second.node_type);
	if (node_found == server_lists_.end())
	{
		return;
	}

	auto child_found = node_found->second.child_lists.find(found->second.child_id);
	if (child_found == node_found->second.child_lists.end())
	{
		return;
	}

	auto &session_lists = child_found->second.session_lists;
	auto iter = std::find(session_lists.begin(), session_lists.end(), session.SessionID());
	if (iter != session_lists.end())
	{
		session_lists.erase(iter);
	}

	if (session_lists.empty())
	{
		node_found->second.child_lists.erase(child_found);
	}

	if (node_found->second.child_lists.empty())
	{
		server_lists_.erase(node_found);
	}

	node_index_.erase(session.SessionID());
}

// 服务器登录
void RouterManager::OnServerLogin(network::TCPSessionHandler &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	// 检查服务器类型是否合法
	auto request = static_cast<internal::LoginRouterReq*>(message);
	if (request->type() < internal::NodeType_MIN || request->type() > internal::NodeType_MAX)
	{
		RespondErrorCode(session, internal::kInvalidNodeType, request->GetTypeName().c_str());
		return;
	}

	// 是否重复登录
	auto found = node_index_.find(session.SessionID());
	if (found != node_index_.end())
	{
		RespondErrorCode(session, internal::kInvalidNodeType, request->GetTypeName().c_str());
		return;
	}

	// 服务器节点登录
	auto node_found = server_lists_.find(request->type());
	if (node_found == server_lists_.end())
	{
		ServerNode &node_lists = server_lists_[request->type()];
		node_lists.node_type = request->type();

		ChildNode &child_node_lists = node_lists.child_lists[request->child_id()];;
		child_node_lists.node_type = request->type();
		child_node_lists.child_id = request->child_id();
		child_node_lists.session_lists.push_back(session.SessionID());
	}
	else
	{
		auto child_found = node_found->second.child_lists.find(request->child_id());
		if (child_found == node_found->second.child_lists.end())
		{
			ChildNode &child_node_lists = node_found->second.child_lists[request->child_id()];
			child_node_lists.node_type = request->type();
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

	// 返回结果
	buffer.Clear();
	internal::LoginRouterRsp rsp;
	rsp.set_heartbeat_interval(60);
	PackageMessage(&rsp, buffer);
	session.Send(buffer);
}

// 转发服务器消息
void RouterManager::OnForwardServerMessage(network::TCPSessionHandler &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	auto request = static_cast<internal::ForwardMessageReq*>(message);
	if (request->message_length() != buffer.Readable())
	{
		RespondErrorCode(session, internal::kInvalidDataPacket);
		return;
	}

	ChildNode *child_node = nullptr;
	if (!FindServerNodeBySessionID(session.SessionID(), child_node))
	{
		RespondErrorCode(session, internal::kNotLoggedIn, request->GetTypeName().c_str());
	}
	else
	{
		network::SessionHandlePointer dst_session;
		if (!FindServerNodeSession(request->dst_type(), request->dst_child_id(), dst_session))
		{
			RespondErrorCode(session, internal::kDestinationUnreachable, request->GetTypeName().c_str());
		}

		network::NetMessage msg;
		internal::ForwardMessageRsp rsp;
		rsp.set_src_type(static_cast<internal::NodeType>(child_node->node_type));
		rsp.set_src_child_id(child_node->child_id);
		rsp.set_message_length(buffer.Readable());
		PackageMessage(&rsp, msg);
		msg.Write(buffer.Data(), buffer.Readable());
		dst_session->Send(msg);
	}
}

// 广播服务器消息
void RouterManager::OnBroadcastServerMessage(network::TCPSessionHandler &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	auto request = static_cast<internal::BroadcastMessageReq*>(message);
	if (request->message_length() != buffer.Readable())
	{
		RespondErrorCode(session, internal::kInvalidDataPacket);
		return;
	}

	ChildNode *child_node = nullptr;
	if (!FindServerNodeBySessionID(session.SessionID(), child_node))
	{
		RespondErrorCode(session, internal::kNotLoggedIn, request->GetTypeName().c_str());
	}
	else
	{
		network::NetMessage msg;
		internal::BroadcastMessageRsp rsp;
		network::SessionHandlePointer dst_session;
		for (int i = 0; i < request->dst_lists().size(); ++i)
		{
			auto node_found = server_lists_.find(request->dst_lists(i));
			if (node_found != server_lists_.end())
			{
				for (auto &pair : node_found->second.child_lists)
				{
					network::TCPSessionID session_id = 0;
					auto &session_lists = pair.second.session_lists;
					if (session_lists.empty())
					{
						continue;
					}

					if (session_lists.size() == 1)
					{
						session_id = session_lists.front();
					}
					else
					{
						session_id = session_lists[rand() % session_lists.size()];
					}

					dst_session = threads_.SessionHandler(session_id);
					if (dst_session != nullptr)
					{	
						rsp.set_src_type(static_cast<internal::NodeType>(child_node->node_type));
						rsp.set_src_child_id(child_node->child_id);
						rsp.set_message_length(buffer.Readable());
						PackageMessage(&rsp, msg);
						msg.Write(buffer.Data(), buffer.Readable());
						dst_session->Send(msg);
						msg.Clear();
					}
				}
			}
		}
	}
}
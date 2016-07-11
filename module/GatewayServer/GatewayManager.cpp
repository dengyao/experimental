#include "GatewayManager.h"
#include <proto/MessageHelper.h>
#include <proto/internal.protocol.pb.h>

template <typename T>
struct Protocol
{
	static std::string name()
	{
		return T::descriptor()->full_name();
	}
};

#define make_handler(func_ptr, obj_ptr) (std::bind(func_ptr, obj_ptr, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))

GatewayManager::GatewayManager(eddy::IOServiceThreadManager &threads)
	: threads_(threads)
{
	msg_handler_.insert(std::make_pair(Protocol<internal::LoginDBProxyReq>::name(), make_handler(&GatewayManager::OnServerLogin, this)));
	msg_handler_.insert(std::make_pair(Protocol<internal::PauseWorkReq>::name(), make_handler(&GatewayManager::OnServerPauseWork, this)));
	msg_handler_.insert(std::make_pair(Protocol<internal::ContinueWorkReq>::name(), make_handler(&GatewayManager::OnServerContinueWork, this)));
	msg_handler_.insert(std::make_pair(Protocol<internal::ForwardMessageReq>::name(), make_handler(&GatewayManager::OnForwardServerMessage, this)));
	msg_handler_.insert(std::make_pair(Protocol<internal::BroadcastMessageReq>::name(), make_handler(&GatewayManager::OnBroadcastServerMessage, this)));
}

GatewayManager::~GatewayManager()
{
}

// 回复错误码
void GatewayManager::RespondErrorCode(eddy::TCPSessionHandler &session, int error_code, const char *what)
{
	internal::GatewayErrorRsp error;
	error.set_error_code(static_cast<internal::ErrorCode>(error_code));
	if (what != nullptr)
	{
		error.set_what(what);
	}
	eddy::NetMessage message;
	PackageMessage(&error, message);
	session.Send(message);
}

// 查找服务器节点
bool GatewayManager::FindServerNodeBySessionID(eddy::TCPSessionID session_id, ChildNode *&out_child_node)
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

// 查找节点会话
bool GatewayManager::FindServerNodeSession(int node_type, int child_id, eddy::SessionHandlePointer &out_session)
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

	eddy::TCPSessionID session_id = 0;
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

// 收到消息
void GatewayManager::OnMessage(eddy::TCPSessionHandler &session, google::protobuf::Message *message, eddy::NetMessage &buffer)
{
	auto found = msg_handler_.find(message->GetTypeName());
	if (found == msg_handler_.end())
	{
		RespondErrorCode(session, internal::kInvalidProtocol);
	}
	found->second(session, message, buffer);
}

// 服务器登录
void GatewayManager::OnServerLogin(eddy::TCPSessionHandler &session, google::protobuf::Message *message, eddy::NetMessage &buffer)
{
	auto request = dynamic_cast<internal::LoginGatewayReq*>(message);
	assert(request != nullptr);
	if (request == nullptr)
	{
		RespondErrorCode(session, internal::kInvalidProtocol);
		return;
	}

	/* 记得检查服务器类型是否合法 */

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
			child_node_lists.working = true;
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
	eddy::NetMessage msg;
	internal::LoginGatewayRsp rsp;
	rsp.set_heartbeat_interval(60);
	PackageMessage(&rsp, msg);
	session.Send(msg);
}

// 服务器暂停服务
void GatewayManager::OnServerPauseWork(eddy::TCPSessionHandler &session, google::protobuf::Message *message, eddy::NetMessage &buffer)
{
	auto request = dynamic_cast<internal::PauseWorkReq*>(message);
	assert(request != nullptr);
	if (request == nullptr)
	{
		RespondErrorCode(session, internal::kInvalidProtocol);
		return;
	}

	ChildNode *child_node = nullptr;
	if (!FindServerNodeBySessionID(session.SessionID(), child_node))
	{
		RespondErrorCode(session, internal::kNotLoggedIn, request->GetTypeName().c_str());
	}
	else
	{
		child_node->working = false;

		// 返回结果
		eddy::NetMessage msg;
		internal::PauseWorkRsp rsp;
		PackageMessage(&rsp, msg);
		session.Send(msg);
	}
}

// 服务器继续服务器
void GatewayManager::OnServerContinueWork(eddy::TCPSessionHandler &session, google::protobuf::Message *message, eddy::NetMessage &buffer)
{
	auto request = dynamic_cast<internal::ContinueWorkReq*>(message);
	assert(request != nullptr);
	if (request == nullptr)
	{
		RespondErrorCode(session, internal::kInvalidProtocol);
		return;
	}

	ChildNode *child_node = nullptr;
	if (!FindServerNodeBySessionID(session.SessionID(), child_node))
	{
		RespondErrorCode(session, internal::kNotLoggedIn, request->GetTypeName().c_str());
	}
	else
	{
		child_node->working = true;

		// 返回结果
		eddy::NetMessage msg;
		internal::ContinueWorkRsp rsp;
		PackageMessage(&rsp, msg);
		session.Send(msg);
	}
}

// 转发服务器消息
void GatewayManager::OnForwardServerMessage(eddy::TCPSessionHandler &session, google::protobuf::Message *message, eddy::NetMessage &buffer)
{
	auto request = dynamic_cast<internal::ForwardMessageReq*>(message);
	assert(request != nullptr);
	if (request == nullptr)
	{
		RespondErrorCode(session, internal::kInvalidProtocol);
		return;
	}

	ChildNode *child_node = nullptr;
	if (!FindServerNodeBySessionID(session.SessionID(), child_node))
	{
		RespondErrorCode(session, internal::kNotLoggedIn, request->GetTypeName().c_str());
	}
	else
	{
		if (child_node->working)
		{
			eddy::SessionHandlePointer dst_session;
			if (!FindServerNodeSession(request->dst_type(), request->dst_child_id(), dst_session))
			{
				RespondErrorCode(session, internal::kDestinationUnreachable, request->GetTypeName().c_str());
			}
			else
			{
				eddy::NetMessage msg;
				internal::ForwardMessageRsp rsp;
				rsp.set_src_type(static_cast<internal::NodeType>(child_node->node_type));
				rsp.set_src_child_id(child_node->child_id);
				PackageMessage(&rsp, msg);
				dst_session->Send(msg);
			}
		}
	}
}

// 广播服务器消息
void GatewayManager::OnBroadcastServerMessage(eddy::TCPSessionHandler &session, google::protobuf::Message *message, eddy::NetMessage &buffer)
{
	auto request = dynamic_cast<internal::BroadcastMessageReq*>(message);
	assert(request != nullptr);
	if (request == nullptr)
	{
		RespondErrorCode(session, internal::kInvalidProtocol);
		return;
	}

	ChildNode *child_node = nullptr;
	if (!FindServerNodeBySessionID(session.SessionID(), child_node))
	{
		RespondErrorCode(session, internal::kNotLoggedIn, request->GetTypeName().c_str());
	}
	else
	{

	}
}
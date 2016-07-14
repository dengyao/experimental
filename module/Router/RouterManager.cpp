#include "RouterManager.h"
#include <iostream>
#include <ProtobufCodec.h>
#include <proto/internal.pb.h>
#include "SessionHandle.h"
#include "StatisticalTools.h"

RouterManager::RouterManager(network::IOServiceThreadManager &threads)
	: threads_(threads)
	, timer_(threads.MainThread()->IOService(), std::chrono::seconds(1))
	, wait_handler_(std::bind(&RouterManager::UpdateStatisicalData, this, std::placeholders::_1))
{
	timer_.async_wait(wait_handler_);
}

RouterManager::~RouterManager()
{
}

// 更新统计数据
void RouterManager::UpdateStatisicalData(asio::error_code error_code)
{
	if (error_code)
	{
		std::cerr << error_code.message() << std::endl;
		return;
	}
	StatisticalTools::GetInstance()->Flush();
	std::cout << "每秒上行流量：" << StatisticalTools::GetInstance()->UpVolume() << "字节" << std::endl;
	std::cout << "每秒下行流量：" << StatisticalTools::GetInstance()->UpVolume() << "字节" << std::endl;
	timer_.expires_from_now(std::chrono::seconds(1));
	timer_.async_wait(wait_handler_);
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

// 回复错误码
void RouterManager::RespondErrorCode(SessionHandle &session, network::NetMessage &buffer, int error_code, const char *what)
{
	buffer.Clear();
	internal::RouterErrorRsp response;
	response.set_error_code(static_cast<internal::ErrorCode>(error_code));
	if (what != nullptr)
	{
		response.set_what(what);
	}
	ProtubufCodec::Encode(&response, buffer);
	session.Respond(buffer);
}

// 处理收到消息
void RouterManager::HandleMessage(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	if (dynamic_cast<internal::LoginRouterReq*>(message) != nullptr)
	{
		OnServerLogin(session, message, buffer);
	}
	else if (dynamic_cast<internal::ForwardReq*>(message) != nullptr)
	{
		OnForwardServerMessage(session, message, buffer);
	}
	else if (dynamic_cast<internal::BroadcastReq*>(message) != nullptr)
	{
		OnBroadcastServerMessage(session, message, buffer);
	}
	else if (dynamic_cast<internal::RouterInfoReq*>(message) != nullptr)
	{
		OnQueryRouterInfo(session, message, buffer);
	}
	else
	{
		RespondErrorCode(session, buffer, internal::kInvalidProtocol, message->GetTypeName().c_str());
	}
}

// 处理服务器下线
void RouterManager::HandleServerOffline(SessionHandle &session)
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
void RouterManager::OnServerLogin(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	// 检查服务器类型是否合法
	auto request = static_cast<internal::LoginRouterReq*>(message);
	if (request->type() < internal::NodeType_MIN || request->type() > internal::NodeType_MAX)
	{
		RespondErrorCode(session, buffer, internal::kInvalidNodeType, request->GetTypeName().c_str());
		return;
	}

	// 是否重复登录
	auto found = node_index_.find(session.SessionID());
	if (found != node_index_.end())
	{
		RespondErrorCode(session, buffer, internal::kInvalidNodeType, request->GetTypeName().c_str());
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
	internal::LoginRouterRsp response;
	response.set_heartbeat_interval(60);
	ProtubufCodec::Encode(&response, buffer);
	session.Respond(buffer);
}

// 查询路由信息
void RouterManager::OnQueryRouterInfo(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	buffer.Clear();
	internal::RouterInfoRsp response;
	response.set_up_volume(StatisticalTools::GetInstance()->UpVolume());
	response.set_down_volume(StatisticalTools::GetInstance()->DownVolume());
	ProtubufCodec::Encode(&response, buffer);
	session.Respond(buffer);
}

// 转发服务器消息
void RouterManager::OnForwardServerMessage(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	auto request = static_cast<internal::ForwardReq*>(message);
	if (request->message_length() != buffer.Readable())
	{
		RespondErrorCode(session, buffer, internal::kInvalidDataPacket);
		return;
	}

	ChildNode *child_node = nullptr;
	if (!FindServerNodeBySessionID(session.SessionID(), child_node))
	{
		RespondErrorCode(session, buffer, internal::kNotLoggedIn, request->GetTypeName().c_str());
		return;
	}

	network::SessionHandlePointer dst_session;
	if (!FindServerNodeSession(request->dst_type(), request->dst_child_id(), dst_session))
	{
		RespondErrorCode(session, buffer, internal::kDestinationUnreachable, request->GetTypeName().c_str());
		return;
	}

	network::NetMessage new_message;
	internal::RouterNotify response;
	response.set_src_type(static_cast<internal::NodeType>(child_node->node_type));
	response.set_src_child_id(child_node->child_id);
	response.set_message_length(buffer.Readable());
	ProtubufCodec::Encode(&response, new_message);
	new_message.Write(buffer.Data(), buffer.Readable());
	static_cast<SessionHandle*>(dst_session.get())->Respond(new_message);
}

// 广播服务器消息
void RouterManager::OnBroadcastServerMessage(SessionHandle &session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	auto request = static_cast<internal::BroadcastReq*>(message);
	if (request->message_length() != buffer.Readable())
	{
		RespondErrorCode(session, buffer, internal::kInvalidDataPacket);
		return;
	}

	ChildNode *child_node = nullptr;
	if (!FindServerNodeBySessionID(session.SessionID(), child_node))
	{
		RespondErrorCode(session, buffer, internal::kNotLoggedIn, request->GetTypeName().c_str());
	}
	
	network::NetMessage new_message;
	internal::RouterNotify response;
	response.set_src_type(static_cast<internal::NodeType>(child_node->node_type));
	response.set_src_child_id(child_node->child_id);
	response.set_message_length(buffer.Readable());
	ProtubufCodec::Encode(&response, new_message);
	new_message.Write(buffer.Data(), buffer.Readable());

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
					static_cast<SessionHandle*>(dst_session.get())->Respond(new_message);
				}
			}
		}
	}
}
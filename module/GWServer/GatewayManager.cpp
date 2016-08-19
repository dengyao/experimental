#include "GatewayManager.h"
#include <ProtobufCodec.h>
#include <proto/public_struct.pb.h>
#include <proto/server_internal.pb.h>
#include "Logging.h"
#include "ServerConfig.h"
#include "SessionHandle.h"
#include "StatisticalTools.h"

using namespace std::placeholders;

GatewayManager::GatewayManager(network::IOServiceThreadManager &threads)
	: threads_(threads)
	, timer_(threads.MainThread()->IOService(), std::chrono::seconds(1))
	, wait_handler_(std::bind(&GatewayManager::UpdateStatisicalData, this, std::placeholders::_1))
	, dispatcher_(std::bind(&GatewayManager::OnUnknownMessage, this, _1, _2, _3))
{
	dispatcher_.RegisterMessageCallback<svr::ForwardReq>(
		std::bind(&GatewayManager::OnForwardServerMessage, this, _1, _2, _3));
	dispatcher_.RegisterMessageCallback<svr::BroadcastReq>(
		std::bind(&GatewayManager::OnBroadcastServerMessage, this, _1, _2, _3));
	dispatcher_.RegisterMessageCallback<svr::GWInfoReq>(
		std::bind(&GatewayManager::OnQueryRouterInfo, this, _1, _2, _3));
	dispatcher_.RegisterMessageCallback<svr::LoginGWReq>(
		std::bind(&GatewayManager::OnServerLogin, this, _1, _2, _3));

	timer_.async_wait(wait_handler_);
}

GatewayManager::~GatewayManager()
{
}

// 更新统计数据
void GatewayManager::UpdateStatisicalData(asio::error_code error_code)
{
	if (error_code)
	{
		logger()->error("{}:{} {}", __FUNCTION__, __LINE__, error_code.message());
		return;
	}

	StatisticalTools::GetInstance()->Flush();
	logger()->info("服务器信息：上行流量{}，下行流量{}",
		StatisticalTools::GetInstance()->UpVolume(),
		StatisticalTools::GetInstance()->DownVolume());

	timer_.expires_from_now(std::chrono::seconds(1));
	timer_.async_wait(wait_handler_);
}

// 查找服务器节点
bool GatewayManager::FindServerNode(int node_type, int child_id, ChildNode *&out_child_node)
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

bool GatewayManager::FindServerNodeBySessionID(network::TCPSessionID session_id, ChildNode *&out_child_node)
{
	auto found = node_index_.find(session_id);
	if (found == node_index_.end())
	{
		return false;
	}
	return FindServerNode(found->second.node_type, found->second.child_id, out_child_node);
}

// 查找节点会话
bool GatewayManager::FindServerNodeSession(int node_type, int child_id, network::SessionHandlePointer &out_session)
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
void GatewayManager::SendErrorCode(SessionHandle *session, network::NetMessage &buffer, int error_code, const char *what)
{
	buffer.Clear();
	pub::ErrorRsp response;
	response.set_error_code(static_cast<pub::ErrorCode>(error_code));
	if (what != nullptr)
	{
		response.set_what(what);
	}
	ProtubufCodec::Encode(&response, buffer);
	session->Write(buffer);
}

// 接收消息
bool GatewayManager::OnMessage(SessionHandle *session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	return dispatcher_.OnProtobufMessage(session, message, buffer);
}

// 连接关闭
void GatewayManager::OnClose(SessionHandle *session)
{
	auto found = node_index_.find(session->SessionID());
	if (found == node_index_.end())
	{
		return;
	}

	const int node_type = found->second.node_type;
	const int child_id = found->second.child_id;
	auto node_found = server_lists_.find(node_type);
	if (node_found == server_lists_.end())
	{
		return;
	}

	auto child_found = node_found->second.child_lists.find(child_id);
	if (child_found == node_found->second.child_lists.end())
	{
		return;
	}

	auto &session_lists = child_found->second.session_lists;
	auto iter = std::find(session_lists.begin(), session_lists.end(), session->SessionID());
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

	node_index_.erase(session->SessionID());

	logger()->info("服务器节点[{},{}]离线，来自{}:{}", node_type, child_id,
		session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
}

// 未定义消息
bool GatewayManager::OnUnknownMessage(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	SendErrorCode(static_cast<SessionHandle*>(session), buffer, pub::kInvalidProtocol, message->GetTypeName().c_str());
	logger()->warn("协议无效，来自{}:{}", session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
	return true;
}

// 服务器登录
bool GatewayManager::OnServerLogin(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	// 检查服务器类型是否合法
	auto request = static_cast<svr::LoginGWReq*>(message);
	if (request->node().type() < svr::NodeType_MIN || request->node().type() > svr::NodeType_MAX)
	{
		SendErrorCode(static_cast<SessionHandle*>(session), buffer, pub::kInvalidNodeType, request->GetTypeName().c_str());
		logger()->warn("无效的服务器节类型{}，来自{}:{}", request->node().type(), session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
		return false;
	}

	// 是否重复登录
	auto found = node_index_.find(session->SessionID());
	if (found != node_index_.end())
	{
		SendErrorCode(static_cast<SessionHandle*>(session), buffer, pub::kInvalidNodeType, request->GetTypeName().c_str());
		logger()->warn("服务器节点重复登录，来自{}:{}", session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
		return false;
	}

	// 服务器节点登录
	auto node_found = server_lists_.find(request->node().type());
	if (node_found == server_lists_.end())
	{
		ServerNode &node_lists = server_lists_[request->node().type()];
		node_lists.node_type = request->node().type();

		ChildNode &child_node_lists = node_lists.child_lists[request->node().child_id()];;
		child_node_lists.node_type = request->node().type();
		child_node_lists.child_id = request->node().child_id();
		child_node_lists.session_lists.push_back(session->SessionID());
	}
	else
	{
		auto child_found = node_found->second.child_lists.find(request->node().child_id());
		if (child_found == node_found->second.child_lists.end())
		{
			ChildNode &child_node_lists = node_found->second.child_lists[request->node().child_id()];
			child_node_lists.node_type = request->node().type();
			child_node_lists.child_id = request->node().child_id();
			child_node_lists.session_lists.push_back(session->SessionID());
		}
		else
		{
			child_found->second.session_lists.push_back(session->SessionID());
		}
	}

	// 新建节点索引
	NodeIndex index;
	index.node_type = request->node().type();
	index.child_id = request->node().child_id();
	node_index_.insert(std::make_pair(session->SessionID(), index));

	// 返回结果
	buffer.Clear();
	svr::LoginGWRsp response;
	response.set_heartbeat_interval(ServerConfig::GetInstance()->GetHeartbeatInterval());
	ProtubufCodec::Encode(&response, buffer);
	static_cast<SessionHandle*>(session)->Write(buffer);

	logger()->info("服务器节点[{},{}]登录成功，来自{}:{}", request->node().type(), request->node().child_id(),
		session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());

	return true;
}

// 查询路由信息
bool GatewayManager::OnQueryRouterInfo(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	buffer.Clear();
	svr::GWInfoRsp response;
	response.set_up_volume(StatisticalTools::GetInstance()->UpVolume());
	response.set_down_volume(StatisticalTools::GetInstance()->DownVolume());
	for (const auto &node : server_lists_)
	{
		for (const auto &child : node.second.child_lists)
		{
			auto node = response.add_node_lists();
			node->set_type(static_cast<svr::NodeType>(child.second.node_type));
			node->set_child_id(child.second.child_id);
		}
	}
	ProtubufCodec::Encode(&response, buffer);
	static_cast<SessionHandle*>(session)->Write(buffer);
	return true;
}

// 转发服务器消息
bool GatewayManager::OnForwardServerMessage(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	ChildNode *child_node = nullptr;
	auto request = static_cast<svr::ForwardReq*>(message);
	if (!FindServerNodeBySessionID(session->SessionID(), child_node))
	{
		SendErrorCode(static_cast<SessionHandle*>(session), buffer, pub::kNotLoggedIn, request->GetTypeName().c_str());
		logger()->warn("服务器节点未登录，来自{}:{}", session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
		return false;
	}

	network::SessionHandlePointer dst_session;
	if (!FindServerNodeSession(request->dst().type(), request->dst().child_id(), dst_session))
	{
		SendErrorCode(static_cast<SessionHandle*>(session), buffer, pub::kDestinationUnreachable, request->GetTypeName().c_str());
		logger()->warn("目标服务器节点[{},{}]不可到达，来自{}:{}", request->dst().type(), request->dst().child_id(),
			session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
		return false;
	}

	buffer.Clear();
	svr::GWNotify response;
	response.mutable_src()->set_type(static_cast<svr::NodeType>(child_node->node_type));
	response.mutable_src()->set_child_id(child_node->child_id);
	response.set_user_data(request->user_data().c_str(), request->user_data().size());
	ProtubufCodec::Encode(&response, buffer);
	static_cast<SessionHandle*>(dst_session.get())->Write(buffer);
	return true;
}

// 广播服务器消息
bool GatewayManager::OnBroadcastServerMessage(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer)
{
	ChildNode *child_node = nullptr;
	auto request = static_cast<svr::BroadcastReq*>(message);
	if (!FindServerNodeBySessionID(session->SessionID(), child_node))
	{
		SendErrorCode(static_cast<SessionHandle*>(session), buffer, pub::kNotLoggedIn, request->GetTypeName().c_str());
		logger()->warn("服务器节点未登录，来自{}:{}", session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
		return false;
	}
	
	buffer.Clear();
	svr::GWNotify response;
	response.mutable_src()->set_type(static_cast<svr::NodeType>(child_node->node_type));
	response.mutable_src()->set_child_id(child_node->child_id);
	response.set_user_data(request->user_data().c_str(), request->user_data().size());
	ProtubufCodec::Encode(&response, buffer);

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
					static_cast<SessionHandle*>(dst_session.get())->Write(buffer);
				}
			}
		}
	}
	return true;
}
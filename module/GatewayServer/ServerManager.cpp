#include "ServerManager.h"


ServerManager::ServerManager()
{

}

ServerManager::~ServerManager()
{
}

// 服务器登录
void ServerManager::OnServerLogin(int node_type, int child_id, google::protobuf::Message *message)
{
	node_lists_[node_type].child_lists[child_id].child_id = child_id;
	node_lists_[node_type].child_lists[child_id].working = true;
	node_lists_[node_type].child_lists[child_id].session_lists.push_back(seeiod);
}

// 服务器暂停服务
void ServerManager::OnServerPauseWork(int node_type, int child_id, google::protobuf::Message *message)
{
	node_lists_[node_type].child_lists[child_id].working = false;
}

// 服务器继续服务器
void ServerManager::OnServerContinueWor(int node_type, int child_id, google::protobuf::Message *message)
{
	node_lists_[node_type].child_lists[child_id].working = true;
}

// 转发服务器服务器消息
void ServerManager::OnForwardServerMessage(int src_node_type, int src_child_id, int dst_node_type, int dst_child_id, google::protobuf::Message *message)
{

}

// 广播服务器消息
void ServerManager::OnBroadcastServerMessage(int src_node_type, int src_child_id, int dst_node_type, int dst_child_id, google::protobuf::Message *message)
{

}
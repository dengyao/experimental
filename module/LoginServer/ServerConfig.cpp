#include "ServerConfig.h"
#include <fstream>
#include <rapidjson/document.h>


ServerConfig::ServerConfig()
	: port_(0)
	, thread_num_(0)
	, dba_port_(0)
	, dba_connections_(0)
	, heartbeat_interval_(0)
	, max_user_online_time_(0)
{
}

ServerConfig::~ServerConfig()
{
}

// 获取服务器端口号
unsigned short ServerConfig::GetPort() const
{
	return port_;
}

// 获取使用线程数量
unsigned short ServerConfig::GetUseThreadNum() const
{
	return thread_num_;
}

// 获取会话心跳间隔
unsigned int ServerConfig::GetHeartbeatInterval() const
{
	return heartbeat_interval_;
}

// 获取用户最大在线时长
unsigned int ServerConfig::GetMaxUserOnlineTime() const
{
	return max_user_online_time_;
}

// 获取数据库代理服务器ip
const char* ServerConfig::GetDBAgentIP() const
{
	return dba_ip_.c_str();
}

// 获取数据库代理服务器端口
unsigned short ServerConfig::GetDBAgentPort() const
{
	return dba_port_;
}

// 获取与数据库代理服务器最大连接数
unsigned short ServerConfig::GetDBAgentConnections() const
{
	return dba_connections_;
}

// 加载服务器配置文件
bool ServerConfig::Load(const std::string &filename)
{
	// 读取文件数据
	std::ifstream file;
	file.open(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		return false;
	}
	std::string buffer;
	buffer.resize(static_cast<size_t>(file.tellg()));
	file.seekg(0);
	file.read(const_cast<char*>(buffer.data()), buffer.size());

	// 解析配置文件
	rapidjson::Document document;
	document.Parse<0>(buffer.data());
	if (document.HasParseError())
	{
		return false;
	}

	if (!document.HasMember("port") || !document["port"].IsUint())
	{
		return false;
	}
	port_ = document["port"].GetUint();

	if (!document.HasMember("thread_num") || !document["thread_num"].IsUint())
	{
		return false;
	}
	thread_num_ = document["thread_num"].GetUint();

	if (!document.HasMember("heartbeat_interval") || !document["heartbeat_interval"].IsUint())
	{
		return false;
	}
	heartbeat_interval_ = document["heartbeat_interval"].GetUint();

	if (!document.HasMember("max_user_online_time") || !document["max_user_online_time"].IsUint())
	{
		return false;
	}
	max_user_online_time_ = document["max_user_online_time"].GetUint();

	if (!document.HasMember("dba_ip") || !document["dba_ip"].IsString())
	{
		return false;
	}
	dba_ip_ = document["dba_ip"].GetString();

	if (!document.HasMember("dba_port") || !document["dba_port"].IsUint())
	{
		return false;
	}
	dba_port_ = document["dba_port"].GetUint();

	if (!document.HasMember("dba_connections") || !document["dba_connections"].IsUint())
	{
		return false;
	}
	dba_connections_ = document["dba_connections"].GetUint();

	return true;
}

// 获取进程名
const char* ServerConfig::ProcessName() const
{
	return process_name_.c_str();
}

// 设置进程名
void ServerConfig::ProcessName(const std::string &name)
{
	process_name_ = name;
}
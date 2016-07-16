#include "ServerConfig.h"
#include <fstream>
#include <rapidjson/document.h>


ServerConfig::ServerConfig()
	: port_(0)
	, thread_num_(0)
	, heartbeat_interval_(0)
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

	return true;
}
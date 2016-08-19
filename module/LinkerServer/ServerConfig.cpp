#include "ServerConfig.h"
#include <fstream>
#include <rapidjson/document.h>


ServerConfig::ServerConfig()
	: port_(0)
	, thread_num_(0)
	, gws_port_(0)
	, gws_connections_(0)
	, heartbeat_interval_(0)
	, max_user_verify_time_(0)
	, login_server_port_(0)
	, patition_id_(0)
	, report_interval_(0)
{
}

ServerConfig::~ServerConfig()
{
}

// 获取服务器公网ip
const char* ServerConfig::GetPublicIP() const
{
	return public_ip_.c_str();
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

// 获取用户最大验证时长
unsigned int ServerConfig::GetMaxUserVerifyTime() const
{
	return max_user_verify_time_;
}

// 获取网关服务器IP
const char* ServerConfig::GetGWServerIP() const
{
	return gws_ip_.c_str();
}

// 获取网关服务器端口
unsigned short ServerConfig::GetGWServerPort() const
{
	return gws_port_;
}

// 获取与网关服务器最大连接数
unsigned short ServerConfig::GetGWServerConnections() const
{
	return gws_connections_;
}

// 获取登录服务器ip
const char* ServerConfig::GetLoginSeverIP() const
{
	return login_server_ip_.c_str();
}

// 获取登录服务器端口
unsigned short ServerConfig::GetLoginServerPort() const
{
	return login_server_port_;
}

// 获取所属分区号
unsigned short ServerConfig::GetPatitionID() const
{
	return patition_id_;
}

// 获取上报时长
unsigned int ServerConfig::GetReportInterval() const
{
	return report_interval_;
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

	if (!document.HasMember("public_ip") || !document["public_ip"].IsString())
	{
		return false;
	}
	public_ip_ = document["public_ip"].GetString();

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

	if (!document.HasMember("max_user_verify_time") || !document["max_user_verify_time"].IsUint())
	{
		return false;
	}
	max_user_verify_time_ = document["max_user_verify_time"].GetUint();

	if (!document.HasMember("gws_ip") || !document["gws_ip"].IsString())
	{
		return false;
	}
	gws_ip_ = document["gws_ip"].GetString();

	if (!document.HasMember("gws_port") || !document["gws_port"].IsUint())
	{
		return false;
	}
	gws_port_ = document["gws_port"].GetUint();

	if (!document.HasMember("gws_connections") || !document["gws_connections"].IsUint())
	{
		return false;
	}
	gws_connections_ = document["gws_connections"].GetUint();

	if (!document.HasMember("ls_ip") || !document["ls_ip"].IsString())
	{
		return false;
	}
	login_server_ip_ = document["ls_ip"].GetString();

	if (!document.HasMember("ls_port") || !document["ls_port"].IsUint())
	{
		return false;
	}
	login_server_port_ = document["ls_port"].GetUint();

	if (!document.HasMember("patition_id") || !document["patition_id"].IsUint())
	{
		return false;
	}
	patition_id_ = document["patition_id"].GetUint();

	if (!document.HasMember("report_interval") || !document["report_interval"].IsUint())
	{
		return false;
	}
	report_interval_ = document["report_interval"].GetUint();

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
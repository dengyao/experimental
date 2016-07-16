#include "ServerConfig.h"
#include <fstream>
#include <rapidjson/document.h>


ServerConfig::ServerConfig()
	: port_(0)
	, thread_num_(0)
	, db_thread_num_(0)
	, heartbeat_interval_(0)
	, backlog_(0)
	, connection_backlog_(0)
	, mysql_port_(0)
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

// 获取数据库使用线程数量
unsigned short ServerConfig::GetDBUseThreadNum() const
{
	return db_thread_num_;
}

// 获取会话心跳间隔
unsigned int ServerConfig::GetHeartbeatInterval() const
{
	return heartbeat_interval_;
}

// 获取最大请求积压数
size_t ServerConfig::GetMaxRequestBacklog() const
{
	return backlog_;
}

// 获取单个连接请求最大积压数
size_t ServerConfig::GetMaxConnectionRequestBacklog() const
{
	return connection_backlog_;
}

// 获取MySQL主机地址
const char* ServerConfig::GetMySQLHost() const
{
	return mysql_host_.c_str();
}

// 获取MySQL主机端口
unsigned short ServerConfig::GetMySQLPort() const
{
	return mysql_port_;
}

// 获取MySQL用户名
const char* ServerConfig::GetMySQLUser() const
{
	return mysql_user_.c_str();
}

// 获取MySQL密码
const char* ServerConfig::GetMySQLPassword() const
{
	return mysql_passwd_.c_str();
}

// 获取MySQL数据库连接信息
const std::vector<ServerConfig::ConnectionMySQL>& ServerConfig::GetMySQLConnectionInfo() const
{
	return connections_;
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

	if (!document.HasMember("db_thread_num") || !document["db_thread_num"].IsUint())
	{
		return false;
	}
	db_thread_num_ = document["db_thread_num"].GetUint();

	if (!document.HasMember("heartbeat_interval") || !document["heartbeat_interval"].IsUint())
	{
		return false;
	}
	heartbeat_interval_ = document["heartbeat_interval"].GetUint();

	if (!document.HasMember("backlog") || !document["backlog"].IsUint())
	{
		return false;
	}
	backlog_ = document["backlog"].GetUint();

	if (!document.HasMember("connection_backlog") || !document["connection_backlog"].IsUint())
	{
		return false;
	}
	connection_backlog_ = document["connection_backlog"].GetUint();

	if (!document.HasMember("mysql_host") || !document["mysql_host"].IsString())
	{
		return false;
	}
	mysql_host_ = document["mysql_host"].GetString();

	if (!document.HasMember("mysql_port") || !document["mysql_port"].IsUint())
	{
		return false;
	}
	mysql_port_ = document["mysql_port"].GetUint();

	if (!document.HasMember("mysql_user") || !document["mysql_user"].IsString())
	{
		return false;
	}
	mysql_user_ = document["mysql_user"].GetString();

	if (!document.HasMember("mysql_passwd") || !document["mysql_passwd"].IsString())
	{
		return false;
	}
	mysql_passwd_ = document["mysql_passwd"].GetString();

	connections_.clear();
	if (!document.HasMember("mysql_db_lists") || !document["mysql_db_lists"].IsArray())
	{
		return false;
	}
	const rapidjson::Value &json_array = document["mysql_db_lists"];
	connections_.resize(json_array.Size());
	for (size_t i = 0; i < json_array.Size(); ++i)
	{
		const rapidjson::Value &json_object = json_array[i];
		if (!json_object.HasMember("db_name") || !json_object["db_name"].IsString() ||
			!json_object.HasMember("connections") || !json_object["connections"].IsUint())
		{
			connections_.clear();
			return false;
		}

		connections_[i].db_name = json_object["db_name"].GetString();
		connections_[i].connections = json_object["connections"].GetUint();
	}

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
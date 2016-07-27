#ifndef __SERVER_CONFIG_H__
#define __SERVER_CONFIG_H__

#include <vector>
#include <string>
#include <common/Singleton.h>

class ServerConfig : public Singleton<ServerConfig>
{
	SINGLETON(ServerConfig);

public:
	// 获取服务器端口号
	unsigned short GetPort() const;

	// 获取使用线程数量
	unsigned short GetUseThreadNum() const;

	// 获取会话心跳间隔
	unsigned int GetHeartbeatInterval() const;

	// 获取用户最大在线时长
	unsigned int GetMaxUserOnlineTime() const;

	// 获取数据库代理服务器ip
	const char* GetDBAgentIP() const;

	// 获取数据库代理服务器端口
	unsigned short GetDBAgentPort() const;

	// 获取验证数据库名称
	const char* GetVerifyDBName() const;

	// 获取与数据库代理服务器最大连接数
	unsigned short GetDBAgentConnections() const;

public:
	// 加载服务器配置文件
	bool Load(const std::string &filename);

	// 获取进程名
	const char* ProcessName() const;

	// 设置进程名
	void ProcessName(const std::string &name);

private:
	unsigned short          port_;
	unsigned short          thread_num_;
	unsigned int            heartbeat_interval_;
	unsigned int            max_user_online_time_;
	std::string             dba_ip_;
	unsigned short          dba_port_;
	std::string             verify_db_name_;
	unsigned short          dba_connections_;
	std::string             process_name_;
};

#endif

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

	// 获取网关服务器IP
	const char* GetGWServerIP() const;

	// 获取网关服务器端口
	unsigned short GetGWServerPort() const;

	// 获取与网关服务器最大连接数
	unsigned short GetGWServerConnections() const;

	// 获取数据库代理服务器ip
	const char* GetDBAgentIP() const;

	// 获取数据库代理服务器端口
	unsigned short GetDBAgentPort() const;

	// 获取游戏数据库名称
	const char* GetGameDBName() const;

	// 获取与数据库代理服务器最大连接数
	unsigned short GetDBAgentConnections() const;

	// 获取数据库同步间隔
	unsigned int GetSyncInterval() const;

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
	std::string             gws_ip_;
	unsigned short          gws_port_;
	unsigned short          gws_connections_;
	std::string             dba_ip_;
	unsigned short          dba_port_;
	std::string             game_db_name_;
	unsigned short          dba_connections_;
	std::string             process_name_;
	unsigned int			sync_interval_;
};

#endif

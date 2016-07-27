#ifndef __SERVER_CONFIG_H__
#define __SERVER_CONFIG_H__

#include <vector>
#include <string>
#include <common/Singleton.h>

class ServerConfig : public Singleton<ServerConfig>
{
	SINGLETON(ServerConfig);

public:
	struct ConnectionMySQL
	{
		std::string db_name;
		unsigned short connections;
	};

public:
	// 获取服务器端口号
	unsigned short GetPort() const;

	// 获取使用线程数量
	unsigned short GetUseThreadNum() const;

	// 获取数据库使用线程数量
	unsigned short GetDBUseThreadNum() const;

	// 获取会话心跳间隔
	unsigned int GetHeartbeatInterval() const;

	// 获取最大请求积压数
	size_t GetMaxRequestBacklog() const;

	// 获取单个连接请求最大积压数
	size_t GetMaxConnectionRequestBacklog() const;

	// 获取MySQL主机地址
	const char* GetMySQLHost() const;

	// 获取MySQL主机端口
	unsigned short GetMySQLPort() const;

	// 获取MySQL用户名
	const char* GetMySQLUser() const;

	// 获取MySQL密码
	const char* GetMySQLPassword() const;

	// 获取MySQL字符集
	const char* GetMySQLCharset() const;

	// 获取MySQL数据库连接信息
	const std::vector<ConnectionMySQL>& GetMySQLConnectionInfo() const;

public:
	// 加载服务器配置文件
	bool Load(const std::string &filename);

	// 获取进程名
	const char* ProcessName() const;

	// 设置进程名
	void ProcessName(const std::string &name);

private:
	unsigned short               port_;
	unsigned short               thread_num_;
	unsigned short               db_thread_num_;
	unsigned int                 heartbeat_interval_;
	size_t                       backlog_;
	size_t                       connection_backlog_;
	std::string                  mysql_host_;
	unsigned short               mysql_port_;
	std::string                  mysql_user_;
	std::string                  mysql_passwd_;
	std::string                  mysql_charset_;
	std::vector<ConnectionMySQL> connections_;
	std::string                  process_name_;
};

#endif

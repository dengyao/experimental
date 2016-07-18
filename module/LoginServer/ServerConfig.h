#ifndef __SERVER_CONFIG_H__
#define __SERVER_CONFIG_H__

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

public:
	// 加载服务器配置文件
	bool Load(const std::string &filename);

	// 获取进程名
	const char* ProcessName() const;

	// 设置进程名
	void ProcessName(const std::string &name);

private:
	unsigned short port_;
	unsigned short thread_num_;
	unsigned int heartbeat_interval_;
	std::string process_name_;
};

#endif

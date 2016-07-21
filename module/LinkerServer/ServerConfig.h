#ifndef __SERVER_CONFIG_H__
#define __SERVER_CONFIG_H__

#include <vector>
#include <common/Singleton.h>

class ServerConfig : public Singleton<ServerConfig>
{
	SINGLETON(ServerConfig);

public:
	// 获取服务器公网ip
	const char* GetPublicIP() const;

	// 获取服务器端口号
	unsigned short GetPort() const;

	// 获取使用线程数量
	unsigned short GetUseThreadNum() const;

	// 获取会话心跳间隔
	unsigned int GetHeartbeatInterval() const;

	// 获取用户最大验证时长
	unsigned int GetMaxUserVerifyTime() const;

	// 获取路由服务器IP
	const char* GetRouterIP() const;

	// 获取路由服务器端口
	unsigned short GetRouterPort() const;

	// 获取与路由服务器最大连接数
	unsigned short GetRouterConnections() const;

	// 获取登录服务器ip
	const char* GetLoginSeverIP() const;

	// 获取登录服务器端口
	unsigned short GetLoginServerPort() const;

	// 获取所属分区号
	unsigned short GetPatitionID() const;

	// 获取上报时长
	unsigned int GetReportInterval() const;

public:
	// 加载服务器配置文件
	bool Load(const std::string &filename);

	// 获取进程名
	const char* ProcessName() const;

	// 设置进程名
	void ProcessName(const std::string &name);

private:
	std::string             public_ip_;
	unsigned short          port_;
	unsigned short          thread_num_;
	unsigned int            heartbeat_interval_;
	unsigned int            max_user_verify_time_;
	std::string             router_ip_;
	unsigned short          router_port_;
	unsigned short          router_connections_;
	std::string             login_server_ip_;
	unsigned short          login_server_port_;
	unsigned short          patition_id_;
	unsigned int            report_interval_;
	std::string             process_name_;
};

#endif

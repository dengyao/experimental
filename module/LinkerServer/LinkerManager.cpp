#include "LinkerManager.h"
#include "Logging.h"

LinkerManager::LinkerManager(network::IOServiceThreadManager &threads)
	: threads_(threads)
	, timer_(threads_.MainThread()->IOService(), std::chrono::seconds(1))
	, wait_handler_(std::bind(&LinkerManager::OnUpdateTimer, this, std::placeholders::_1))
{

}

// 处理来自网关的消息
void LinkerManager::HandleMessageFromRouter()
{

}

// 处理来自登录服务器的消息
void LinkerManager::HandleMessageFromLoginServer()
{

}

// 更新定时器
void LinkerManager::OnUpdateTimer(asio::error_code error_code)
{
	if (error_code)
	{
		logger()->error("%s:%d %s", __FUNCTION__, __LINE__, error_code.message());
		return;
	}

	// 删除超时未验证的用户

	timer_.expires_from_now(std::chrono::seconds(1));
	timer_.async_wait(wait_handler_);
}
#include "GlobalObject.h"
#include <atomic>
#include <Connector.h>
#include "LoginConnector.h"

namespace global_stuff
{
	std::unique_ptr<router::Connector> g_connector;
	std::unique_ptr<LoginConnector> g_login_connector;
}

const std::unique_ptr<LoginConnector>& GlobalLoginConnector()
{
	return global_stuff::g_login_connector;
}

void OnceInitGlobalLoginConnector(std::unique_ptr<LoginConnector> &&connector)
{
	static std::atomic_bool initialized = false;
	assert(!initialized);
	assert(connector != nullptr);
	if (initialized || connector == nullptr)
	{
		exit(-1);
	}
	global_stuff::g_login_connector = std::forward<std::unique_ptr<LoginConnector>>(connector);
}

const std::unique_ptr<router::Connector>& GlobalConnector()
{
	return global_stuff::g_connector;
}

void OnceInitGlobalConnector(std::unique_ptr<router::Connector> &&connector)
{
	static std::atomic_bool initialized = false;
	assert(!initialized);
	assert(connector != nullptr);
	if (initialized || connector == nullptr)
	{	
		exit(-1);
	}
	global_stuff::g_connector = std::forward<std::unique_ptr<router::Connector>>(connector);
}
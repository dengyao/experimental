#include "GlobalObject.h"
#include <atomic>
#include <GWClient.h>
#include "LoginConnector.h"

namespace global_stuff
{
	std::unique_ptr<LoginConnector> g_login_connector;
	std::unique_ptr<gw::GWClient> g_gateway_connector;
}

const std::unique_ptr<LoginConnector>& GlobalLoginConnector()
{
	return global_stuff::g_login_connector;
}

void OnceInitGlobalLoginConnector(std::unique_ptr<LoginConnector> &&connector)
{
	static std::atomic_bool initialized;
	assert(!initialized);
	assert(connector != nullptr);
	if (initialized || connector == nullptr)
	{
		exit(-1);
	}
	global_stuff::g_login_connector = std::forward<std::unique_ptr<LoginConnector>>(connector);
}

const std::unique_ptr<gw::GWClient>& GlobalGWClient()
{
	return global_stuff::g_gateway_connector;
}

void OnceInitGlobalGatewayConnector(std::unique_ptr<gw::GWClient> &&connector)
{
	static std::atomic_bool initialized;
	assert(!initialized);
	assert(connector != nullptr);
	if (initialized || connector == nullptr)
	{	
		exit(-1);
	}
	global_stuff::g_gateway_connector = std::forward<std::unique_ptr<gw::GWClient>>(connector);
}
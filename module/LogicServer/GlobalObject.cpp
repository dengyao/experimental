#include "GlobalObject.h"
#include <atomic>
#include <DBClient.h>

namespace global_stuff
{
	std::unique_ptr<db::DBClient> g_db_client;
	std::unique_ptr<gw::GWClient> g_gateway_connector;
}

const std::unique_ptr<db::DBClient>& GlobalDBClient()
{
	return global_stuff::g_db_client;
}

void OnceInitGlobalDBClient(std::unique_ptr<db::DBClient> &&db_client)
{
	static std::atomic_bool initialized;
	assert(!initialized);
	assert(db_client != nullptr);
	if (initialized || db_client == nullptr)
	{
		exit(-1);
	}
	global_stuff::g_db_client = std::forward<std::unique_ptr<db::DBClient>>(db_client);
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
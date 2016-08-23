#ifndef __GLOBAL_OBJECT_H__
#define __GLOBAL_OBJECT_H__

#include <memory>

namespace db
{
	class DBClient;
}

namespace gateway
{
	class GatewayClient;
}

const std::unique_ptr<db::DBClient>& GlobalDBClient();
void OnceInitGlobalDBClient(std::unique_ptr<db::DBClient> &&db_client);

const std::unique_ptr<gateway::GatewayClient>& GlobalGatewayClient();
void OnceInitGlobalGatewayConnector(std::unique_ptr<gateway::GatewayClient> &&connector);


#endif

#ifndef __GLOBAL_OBJECT_H__
#define __GLOBAL_OBJECT_H__

#include <memory>

namespace gateway
{
	class GatewayClient;
}

class LoginConnector;

const std::unique_ptr<LoginConnector> &GlobalLoginConnector();
void OnceInitGlobalLoginConnector(std::unique_ptr<LoginConnector> &&connector);

const std::unique_ptr<gateway::GatewayClient>& GlobalGatewayClient();
void OnceInitGlobalGatewayConnector(std::unique_ptr<gateway::GatewayClient> &&connector);


#endif

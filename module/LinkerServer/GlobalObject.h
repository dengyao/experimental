#ifndef __GLOBAL_OBJECT_H__
#define __GLOBAL_OBJECT_H__

#include <memory>

namespace gw
{
	class GWClient;
}

class LoginConnector;

const std::unique_ptr<LoginConnector> &GlobalLoginConnector();
void OnceInitGlobalLoginConnector(std::unique_ptr<LoginConnector> &&connector);

const std::unique_ptr<gw::GWClient>& GlobalGWClient();
void OnceInitGlobalGatewayConnector(std::unique_ptr<gw::GWClient> &&connector);


#endif

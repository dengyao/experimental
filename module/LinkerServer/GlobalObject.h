#ifndef __GLOBAL_OBJECT_H__
#define __GLOBAL_OBJECT_H__

#include <memory>

namespace router
{
	class Connector;
}

class LoginConnector;

const std::unique_ptr<LoginConnector> &GlobalLoginConnector();
void OnceInitGlobalLoginConnector(std::unique_ptr<LoginConnector> &&connector);

const std::unique_ptr<router::Connector>& GlobalConnector();
void OnceInitGlobalConnector(std::unique_ptr<router::Connector> &&connector);


#endif

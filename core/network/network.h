#ifndef __NETWORK_H__
#define __NETWORK_H__

namespace network
{
	class TCPClient;
	class TCPServer;
	class NetMessage;
	class TCPSessionHandler;
	class DefaultMessageFilter;
	class MessageFilterInterface;
	class IOServiceThreadManager;
}

#include "network/TCPClient.h"
#include "network/TCPServer.h"
#include "network/NetMessage.h"
#include "network/TCPSessionHandler.h"
#include "network/DefaultMessageFilter.h"
#include "network/IOServiceThreadManager.h"

#endif
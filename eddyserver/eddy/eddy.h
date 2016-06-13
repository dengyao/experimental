#ifndef __EDDYSERVER_H__
#define __EDDYSERVER_H__

namespace eddy
{
	class TCPClient;
	class TCPServer;
	class NetMessage;
	class TCPSessionHandle;
	class DefaultMessageFilter;
	class MessageFilterInterface;
	class IOServiceThreadManager;
}

#include <eddy/TCPClient.h>
#include <eddy/TCPServer.h>
#include <eddy/NetMessage.h>
#include <eddy/TCPSessionHandle.h>
#include <eddy/DefaultMessageFilter.h>
#include <eddy/IOServiceThreadManager.h>

#endif
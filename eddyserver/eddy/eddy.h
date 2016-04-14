#ifndef __EDDYSERVER_H__
#define __EDDYSERVER_H__

namespace eddy
{
	class TCPClient;
	class TCPServer;
	class TaskQueue;
	class NetMessage;
    class TaskQueuePool;
	class TCPSessionHandle;
	class SimpleMessageFilter;
	class MessageFilterInterface;
	class IOServiceThreadManager;
}

#include "eddy/TCPClient.h"
#include "eddy/TCPServer.h"
#include "eddy/TaskQueue.h"
#include "eddy/NetMessage.h"
#include "eddy/TCPSessionHandle.h"
#include "eddy/SimpleMessageFilter.h"
#include "eddy/IOServiceThreadManager.h"

#endif
#ifndef __EDDYSERVER_H__
#define __EDDYSERVER_H__

namespace eddy
{
	class TCPClient;
	class TCPServer;
	class TaskQueue;
	class TCPSessionHandle;
	class SimpleMessageFilter;
	class MessageFilterInterface;
	class IOServiceThreadManager;
}

#include "eddy/tcp_client.h"
#include "eddy/tcp_server.h"
#include "eddy/task_queue.h"
#include "eddy/message_filter.h"
#include "eddy/tcp_session_handle.h"
#include "eddy/io_service_thread_manager.h"

#endif
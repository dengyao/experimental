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

#include "eddyserver/tcp_client.h"
#include "eddyserver/tcp_server.h"
#include "eddyserver/task_queue.h"
#include "eddyserver/message_filter.h"
#include "eddyserver/tcp_session_handle.h"
#include "eddyserver/io_service_thread_manager.h"

#endif
#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include "types.h"
#include "id_generator.h"

class TCPSession;
class IOServiceThread;
class TCPSessionHandle;

class IOServiceThreadManager final
{
public:
	typedef std::shared_ptr<TCPSession>									SessionPointer;
	typedef std::shared_ptr<IOServiceThread>							ThreadPointer;
	typedef std::shared_ptr<TCPSessionHandle>							SessionHandlerPointer;
	typedef std::unordered_map<TCPSessionID, SessionHandlerPointer >	SessionHandlerMap;

public:
	explicit IOServiceThreadManager(size_t thread_num = 1);

	~IOServiceThreadManager();

public:
	void run();

	void stop();

	ThreadPointer thread();

	ThreadPointer thread(ThreadID id);

	ThreadPointer main_thread();

	void on_session_connect(SessionPointer session_ptr, SessionHandlerPointer handler);

	void on_session_close(TCPSessionID id);

	SessionHandlerPointer session_handler(TCPSessionID id) const;

protected:
	IOServiceThreadManager(const IOServiceThreadManager&) = delete;
	IOServiceThreadManager& operator=(const IOServiceThreadManager&) = delete;

private:
	std::vector<ThreadPointer>	threads_;
	SessionHandlerMap			session_handler_map_;
	IDGenerator					id_generator_;
};
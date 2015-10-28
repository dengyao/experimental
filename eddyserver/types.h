#pragma once

#include <memory>
#include <thread>
#include <cstdint>


namespace eddy
{
	typedef uint32_t								TCPSessionID;
	typedef std::thread::id							ThreadID;

	class											NetMessage;
	class											TCPSession;
	class											IOServiceThread;
	class											TCPSessionHandle;
	class											MessageFilterInterface;

	typedef std::shared_ptr<NetMessage>				NetMessagePointer;
	typedef std::shared_ptr<TCPSession>				SessionPointer;
	typedef std::shared_ptr<IOServiceThread>		ThreadPointer;
	typedef std::shared_ptr<TCPSessionHandle>		SessionHandlerPointer;
	typedef std::shared_ptr<MessageFilterInterface>	MessageFilterPointer;
}
#ifndef __INTERFACE_CONNECTOR_H__
#define __INTERFACE_CONNECTOR_H__

#include <memory>
#include "AgentTypes.h"

class InterfaceConnector
{
	InterfaceConnector(const InterfaceConnector&) = delete;
	InterfaceConnector& operator= (const InterfaceConnector&) = delete;

public:
	InterfaceConnector(const std::string &host, unsigned short port, const std::string &user, const std::string &passwd)
		: host_(host), port_(port), user_(user), passwd_(passwd), timeout_(0)
	{
	}

	InterfaceConnector(const std::string &host, unsigned short port, const std::string &user, const std::string &passwd, int timeout)
		: host_(host), port_(port), user_(user), passwd_(passwd), timeout_(timeout)
	{
	}

	virtual ~InterfaceConnector() = default;

public:
	virtual const char* Name() const = 0;

	virtual ByteArray Call(const ByteArray &command, ErrorCode &error_code) = 0;

	virtual ByteArray Select(const ByteArray &command, ErrorCode &error_code) = 0;

	virtual ByteArray Insert(const ByteArray &command, ErrorCode &error_code) = 0;

	virtual ByteArray Update(const ByteArray &command, ErrorCode &error_code) = 0;

	virtual ByteArray Delete(const ByteArray &command, ErrorCode &error_code) = 0;

protected:
	const std::string		host_;
	const unsigned short	port_;
	const std::string		user_;
	const std::string		passwd_;
	const unsigned int		timeout_;
};

typedef std::unique_ptr<InterfaceConnector> ConnectorPointer;

#endif
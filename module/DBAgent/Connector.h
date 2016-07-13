#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__

#include "AgentTypes.h"

// Database Result Interface
template <typename Database>
class DBResult
{
public:
	ByteArray& GetData();

	const ByteArray& GetData() const;

	DBResult(DBResult&&);

	DBResult& operator= (DBResult&&);

private:
	DBResult(const DBResult&) = delete;
	DBResult& operator= (const DBResult&) = delete;
};

// Database Connector Interface
template <typename Database>
class Connector
{
public:
	Connector(const std::string &host, unsigned short port, const std::string &user, const std::string &passwd);

	Connector(const std::string &host, unsigned short port, const std::string &user, const std::string &passwd, int timeout);

	const char* Name() const;

	DBResult<Database> Select(const ByteArray &command, ErrorCode &error_code);

	DBResult<Database> Insert(const ByteArray &command, ErrorCode &error_code);

	DBResult<Database> Update(const ByteArray &command, ErrorCode &error_code);

	DBResult<Database> Delete(const ByteArray &command, ErrorCode &error_code);

private:
	Connector(const Connector&) = delete;
	Connector& operator= (const Connector&) = delete;
};

#endif
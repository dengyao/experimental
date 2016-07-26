#ifndef __CONNECTOR_MYSQL_H__
#define __CONNECTOR_MYSQL_H__

#include <mysql.h>
#include "InterfaceConnector.h"

class ConnectorMySQL : public InterfaceConnector
{
public:
	ConnectorMySQL(const std::string &host, unsigned short port, const std::string &user, const std::string &passwd);

	ConnectorMySQL(const std::string &host, unsigned short port, const std::string &user, const std::string &passwd, int timeout);

	~ConnectorMySQL();

public:
	const char* Name() const override;

	ByteArray Select(const ByteArray &command, ErrorCode &error_code) override;

	ByteArray Insert(const ByteArray &command, ErrorCode &error_code) override;

	ByteArray Update(const ByteArray &command, ErrorCode &error_code) override;

	ByteArray Delete(const ByteArray &command, ErrorCode &error_code) override;

public:
	void SelectDatabase(const char *db, ErrorCode &error_code);

	void SetCharacterSet(const char *csname, ErrorCode &error_code);

private:
	void Connect();

	void Connect(int timeout);

	void Disconnect();

	bool IsConnected() const;

	ByteArray AffectedRows(const ByteArray &command, ErrorCode &error_code);

private:
	MYSQL		mysql_;
	std::string	select_db_;
	bool		connected_;
};

#endif
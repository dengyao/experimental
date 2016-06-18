﻿#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__

#include <memory>
#include <DBProxy/Private/Types.h>
#include <DBProxy/Public/ResultWrap.h>

namespace dbproxy
{
	// Database Result Interface
	template <typename Database>
	class DatabaseResult
	{
	public:
		size_t Size() const;

		const char* Binary() const;

		ResultWrap Wrap() const;

		DatabaseResult(DatabaseResult&&);;

		DatabaseResult& operator= (DatabaseResult&&);

	private:
		DatabaseResult(const DatabaseResult&) = delete;
		DatabaseResult& operator= (const DatabaseResult&) = delete;
	};

	// Database Connector Interface
	template <typename Database>
	class Connector : public std::enable_shared_from_this< Connector<Database> >
	{
	public:
		Connector(const std::string &host, unsigned short port, const std::string &user, const std::string &passwd);

		Connector(const std::string &host, unsigned short port, const std::string &user, const std::string &passwd, int timeout);

		DatabaseResult<Database> Select(const std::string &command, ErrorCode &error_code);

		DatabaseResult<Database> Insert(const std::string &command, ErrorCode &error_code);

		DatabaseResult<Database> Update(const std::string &command, ErrorCode &error_code);

		DatabaseResult<Database> Delete(const std::string &command, ErrorCode &error_code);

	private:
		Connector(const Connector&) = delete;
		Connector& operator= (const Connector&) = delete;
	};
}

#endif
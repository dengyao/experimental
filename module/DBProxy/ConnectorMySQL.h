#ifndef __CONNECTOR_MYSQL_H__
#define __CONNECTOR_MYSQL_H__

#include <array>
#include <vector>
#include <memory>
#include <cassert>
#include <mysql.h>
#include <mysqld_error.h>
#include "Connector.h"

namespace dbproxy
{
	class MySQL;

	template <>
	class DatabaseResult<MySQL>
	{
	public:
		DatabaseResult() = default;

		DatabaseResult(MYSQL_RES *result)
		{
			Serialize(result);
		}

		DatabaseResult(const char *data, size_t size)
		{
			binary_.resize(size);
			memcpy(binary_.data(), data, size);
		}

		WrapResult Wrap() const
		{
			return WrapResult(Binary(), Size());
		}

		DatabaseResult(DatabaseResult &&other)
		{
			binary_ = std::move(other.binary_);
		}

		DatabaseResult& operator= (DatabaseResult &&other)
		{
			binary_ = std::move(other.binary_);
			return *this;
		}

	public:
		size_t Size() const
		{
			return binary_.size();
		}

		const char* Binary() const
		{
			return binary_.data();
		}

	private:
		void Serialize(MYSQL_RES *result)
		{
			if (result != nullptr)
			{
				size_t use_size = 0;
				std::array<char, 65535> extrabuf;
				const my_ulonglong num_rows = mysql_num_rows(result);
				const my_ulonglong num_fields = mysql_num_fields(result);
				use_size += snprintf(extrabuf.data(), extrabuf.size(), "%lld", num_rows) + 1;
				use_size += snprintf(extrabuf.data() + use_size, extrabuf.size() - use_size, "%lld", num_fields) + 1;
				for (unsigned int row = 0; row < num_rows; ++row)
				{
					MYSQL_ROW row_data = mysql_fetch_row(result);
					assert(row_data != nullptr);
					for (unsigned int field = 0; field < num_fields; ++field)
					{
						size_t field_size = strlen(row_data[field]) + 1;
						size_t new_use_size = use_size + field_size;
						if (new_use_size > extrabuf.size())
						{
							if (binary_.empty())
							{
								binary_.reserve(row == num_rows - 1 && field == num_fields - 1 ? new_use_size : new_use_size * 2);
								binary_.resize(new_use_size);
								memcpy(binary_.data(), extrabuf.data(), use_size);
								memcpy(binary_.data() + use_size, row_data[field], field_size);
							}
							else
							{
								while (new_use_size > binary_.capacity())
								{
									binary_.reserve(new_use_size * 2);
								}
								binary_.resize(new_use_size);
								memcpy(binary_.data() + use_size, row_data[field], field_size);
							}
						}
						else
						{
							memcpy(extrabuf.data() + use_size, row_data[field], field_size);
						}
						use_size = new_use_size;
					}
				}

				if (binary_.empty())
				{
					binary_.resize(use_size);
					memcpy(binary_.data(), extrabuf.data(), use_size);
				}
			}
		}

	private:
		DatabaseResult(const DatabaseResult&) = delete;
		DatabaseResult& operator= (const DatabaseResult&) = delete;

	private:
		std::vector<char> binary_;
	};

	typedef DatabaseResult<MySQL> MySQLResult;

	template <>
	class Connector<MySQL>
	{
	public:
		Connector(const std::string &host, unsigned short port, const std::string &user, const std::string &passwd)
			: host_(host)
			, port_(port)
			, user_(user)
			, passwd_(passwd)
			, connected_(false)
		{
			Connect();
		}

		Connector(const std::string &host, unsigned short port, const std::string &user, const std::string &passwd, int timeout)
			: host_(host)
			, port_(port)
			, user_(user)
			, passwd_(passwd)
			, connected_(false)
		{
			Connect(timeout);
		}

		~Connector()
		{
			Disconnect();
		}

		const char* Name() const
		{
			return select_db_.c_str();
		}

		void SelectDatabase(const char *db, ErrorCode &error_code)
		{
			if (IsConnected())
			{
				mysql_select_db(&mysql_, db);
				int error = mysql_errno(&mysql_);
				if (error == 0)
				{
					select_db_ = db;
				}
				else
				{
					error_code.SetError(error, mysql_error(&mysql_));
				}
			}
			else
			{
				throw NotConnected();
			}
		}

		void SetCharacterSet(const char *csname, ErrorCode &error_code)
		{
			if (IsConnected())
			{
				mysql_set_character_set(&mysql_, csname);
				int error = mysql_errno(&mysql_);
				if (error != 0)
				{
					error_code.SetError(error, mysql_error(&mysql_));
				}
			}
			else
			{
				throw NotConnected();
			}
		}

		MySQLResult Select(const std::vector<char> &command, ErrorCode &error_code)
		{
			if (IsConnected())
			{
				mysql_real_query(&mysql_, command.data(), command.size());
				int error = mysql_errno(&mysql_);
				if (error != 0)
				{
					error_code.SetError(error, mysql_error(&mysql_));
					return MySQLResult();
				}

				MYSQL_RES *sql_result = mysql_store_result(&mysql_);
				MySQLResult result(sql_result);
				mysql_free_result(sql_result);
				return result;
			}
			else
			{
				throw NotConnected();
			}
		}

		MySQLResult Insert(const std::vector<char> &command, ErrorCode &error_code)
		{
			if (IsConnected())
			{
				mysql_real_query(&mysql_, command.data(), command.size());
				int error = mysql_errno(&mysql_);
				if (error != 0)
				{
					error_code.SetError(error, mysql_error(&mysql_));
					return MySQLResult();
				}

				size_t use_size = 4;
				my_ulonglong id = mysql_insert_id(&mysql_);
				my_ulonglong rows = mysql_affected_rows(&mysql_);
				std::array<char, 64> extrabuf = { '1', '\0', '2', '\0' };
				use_size += snprintf(extrabuf.data() + use_size, extrabuf.size() - use_size, "%lld", id) + 1;
				use_size += snprintf(extrabuf.data() + use_size, extrabuf.size() - use_size, "%lld", rows) + 1;
				return MySQLResult(extrabuf.data(), use_size);
			}
			else
			{
				throw NotConnected();
			}
		}

		MySQLResult Update(const std::vector<char> &command, ErrorCode &error_code)
		{
			return AffectedRows(command, error_code);
		}

		MySQLResult Delete(const std::vector<char> &command, ErrorCode &error_code)
		{
			return AffectedRows(command, error_code);
		}

	private:
		void Connect()
		{
			if (!IsConnected())
			{
				int reconnect = 1;
				mysql_init(&mysql_);
				mysql_options(&mysql_, MYSQL_OPT_RECONNECT, &reconnect);
				connected_ = mysql_real_connect(&mysql_, host_.c_str(), user_.c_str(), passwd_.c_str(), nullptr, port_, nullptr, 0) != nullptr;
				if (mysql_errno(&mysql_) != 0)
				{
					throw ConnectionError();
				}
			}
		}

		void Connect(int timeout)
		{
			if (!IsConnected())
			{
				int reconnect = 1;
				mysql_init(&mysql_);
				mysql_options(&mysql_, MYSQL_OPT_RECONNECT, &reconnect);
				mysql_options(&mysql_, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
				connected_ = mysql_real_connect(&mysql_, host_.c_str(), user_.c_str(), passwd_.c_str(), nullptr, port_, nullptr, 0) != nullptr;
				if (mysql_errno(&mysql_) != 0)
				{
					throw ConnectionError();
				}
			}
		}

		void Disconnect()
		{
			if (connected_)
			{
				mysql_close(&mysql_);
				connected_ = false;
			}
		}

		bool IsConnected() const
		{
			return connected_;
		}

		MySQLResult AffectedRows(const std::vector<char> &command, ErrorCode &error_code)
		{
			if (IsConnected())
			{
				mysql_real_query(&mysql_, command.data(), command.size());
				int error = mysql_errno(&mysql_);
				if (error != 0)
				{
					error_code.SetError(error, mysql_error(&mysql_));
					return MySQLResult();
				}

				size_t use_size = 4;
				my_ulonglong rows = mysql_affected_rows(&mysql_);
				std::array<char, 64> extrabuf = { '1', '\0', '1', '\0' };
				use_size += snprintf(extrabuf.data() + use_size, extrabuf.size() - use_size, "%lld", rows) + 1;
				return MySQLResult(extrabuf.data(), use_size);
			}
			else
			{
				throw NotConnected();
			}
		}

	private:
		Connector(const Connector&) = delete;
		Connector& operator= (const Connector&) = delete;

	private:
		MYSQL					mysql_;
		const std::string		host_;
		const unsigned short	port_;
		const std::string		user_;
		const std::string		passwd_;
		std::string				select_db_;
		bool					connected_;
	};

	typedef Connector<MySQL> ConnectorMySQL;
	typedef std::unique_ptr<ConnectorMySQL> ConnectorMySQLPointer;
}

#endif
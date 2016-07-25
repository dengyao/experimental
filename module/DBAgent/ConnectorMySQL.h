#ifndef __CONNECTOR_MYSQL_H__
#define __CONNECTOR_MYSQL_H__

#include <iostream>

#include <array>
#include <vector>
#include <memory>
#include <numeric>
#include <cassert>
#include <mysql.h>
#include <mysqld_error.h>
#include "Connector.h"

class MySQL;

template <>
class DBResult<MySQL>
{
public:
	DBResult() = default;

	DBResult(MYSQL_RES *result)
	{
		Serialize(result);
	}

	DBResult(const char *data, size_t size)
	{
		data_.resize(size);
		memcpy(data_.data(), data, size);
	}

	ByteArray& GetData()
	{
		return data_;
	}

	const ByteArray& GetData() const
	{
		return data_;
	}

	DBResult(DBResult &&other)
	{
		data_ = std::move(other.data_);
	}

	DBResult& operator= (DBResult &&other)
	{
		data_ = std::move(other.data_);
		return *this;
	}

public:
	size_t Size() const
	{
		return data_.size();
	}

	const char* Binary() const
	{
		return data_.data();
	}

private:
	void Serialize(MYSQL_RES *result)
	{
		if (result != nullptr)
		{
			size_t use_size = 0;
			std::array<char, std::numeric_limits<uint16_t>::max()> extrabuf;

			MYSQL_FIELD *fields = mysql_fetch_fields(result);
			const my_ulonglong num_rows = mysql_num_rows(result) + 1;
			const my_ulonglong num_fields = mysql_num_fields(result);
			use_size += snprintf(extrabuf.data(), extrabuf.size(), "%lld", num_rows) + 1;
			use_size += snprintf(extrabuf.data() + use_size, extrabuf.size() - use_size, "%lld", num_fields) + 1;

			// 首行存放字段名
			for (size_t i = 0; i < num_fields; ++i)
			{
				size_t field_size = strlen(fields[i].name) + 1;
				size_t new_use_size = use_size + field_size;
				if (new_use_size > extrabuf.size())
				{
					if (data_.empty())
					{
						data_.reserve(i == num_fields - 1 ? new_use_size : new_use_size * 2);
						data_.resize(use_size);
						memcpy(data_.data(), extrabuf.data(), use_size);
					}
					else
					{
						while (new_use_size > data_.capacity())
						{
							data_.reserve(new_use_size * 2);
						}
					}
					data_.resize(new_use_size);
					memcpy(data_.data() + use_size, fields[i].name, field_size);
				}
				else
				{
					memcpy(extrabuf.data() + use_size, fields[i].name, field_size);
				}
				use_size = new_use_size;
			}

			// 数据从第二行开始存放
			for (unsigned int row = 1; row < num_rows; ++row)
			{
				MYSQL_ROW row_data = mysql_fetch_row(result);
				assert(row_data != nullptr);
				for (unsigned int field = 0; field < num_fields; ++field)
				{
					bool is_null = row_data[field] == nullptr;
					size_t field_size = (is_null ? 0 : strlen(row_data[field])) + 1;
					size_t new_use_size = use_size + field_size;
					if (new_use_size > extrabuf.size())
					{
						if (data_.empty())
						{
							data_.reserve(row == num_rows - 1 && field == num_fields - 1 ? new_use_size : new_use_size * 2);
							data_.resize(use_size);
							memcpy(data_.data(), extrabuf.data(), use_size);
						}
						else
						{
							while (new_use_size > data_.capacity())
							{
								data_.reserve(new_use_size * 2);
							}
						}
						data_.resize(new_use_size);
						if (is_null)
						{
							data_[use_size] = '\0';
						}
						else
						{
							memcpy(data_.data() + use_size, row_data[field], field_size);
						}
					}
					else
					{
						if (is_null)
						{
							extrabuf.data()[use_size] = '\0';
						}
						else
						{
							memcpy(extrabuf.data() + use_size, row_data[field], field_size);
						}				
					}
					use_size = new_use_size;
				}
			}

			if (data_.empty())
			{
				data_.resize(use_size);
				memcpy(data_.data(), extrabuf.data(), use_size);
			}
		}
	}

private:
	DBResult(const DBResult&) = delete;
	DBResult& operator= (const DBResult&) = delete;

private:
	ByteArray data_;
};

typedef DBResult<MySQL> MySQLResult;

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

	MySQLResult Select(const ByteArray &command, ErrorCode &error_code)
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

	MySQLResult Insert(const ByteArray &command, ErrorCode &error_code)
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

	MySQLResult Update(const ByteArray &command, ErrorCode &error_code)
	{
		return AffectedRows(command, error_code);
	}

	MySQLResult Delete(const ByteArray &command, ErrorCode &error_code)
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
			mysql_set_server_option(&mysql_, MYSQL_OPTION_MULTI_STATEMENTS_ON);
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

	MySQLResult AffectedRows(const ByteArray &command, ErrorCode &error_code)
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

#endif
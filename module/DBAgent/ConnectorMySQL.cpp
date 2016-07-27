#include "ConnectorMySQL.h"
#include <array>
#include <limits>
#include <cassert>
#include <cstring>
#include <mysqld_error.h>
#include "ProcedureMySQL.h"

namespace mysql_stuff
{
	// 序列化MySQL结果集
	bool Serialize(MYSQL_RES *result, ByteArray *out_bytes)
	{
		assert(result != nullptr && out_bytes != nullptr);
		if (result == nullptr || out_bytes == nullptr)
		{
			return false;
		}

		out_bytes->clear();
		size_t use_size = 0;
		std::array<char, (std::numeric_limits<uint16_t>::max)()> extrabuf;

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
				if (out_bytes->empty())
				{
					out_bytes->reserve(i == num_fields - 1 ? new_use_size : new_use_size * 2);
					out_bytes->resize(use_size);
					memcpy(out_bytes->data(), extrabuf.data(), use_size);
				}
				else
				{
					while (new_use_size > out_bytes->capacity())
					{
						out_bytes->reserve(new_use_size * 2);
					}
				}
				out_bytes->resize(new_use_size);
				memcpy(out_bytes->data() + use_size, fields[i].name, field_size);
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
					if (out_bytes->empty())
					{
						out_bytes->reserve(row == num_rows - 1 && field == num_fields - 1 ? new_use_size : new_use_size * 2);
						out_bytes->resize(use_size);
						memcpy(out_bytes->data(), extrabuf.data(), use_size);
					}
					else
					{
						while (new_use_size > out_bytes->capacity())
						{
							out_bytes->reserve(new_use_size * 2);
						}
					}
					out_bytes->resize(new_use_size);
					if (is_null)
					{
						out_bytes->at(use_size) = '\0';
					}
					else
					{
						memcpy(out_bytes->data() + use_size, row_data[field], field_size);
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

		if (out_bytes->empty())
		{
			out_bytes->resize(use_size);
			memcpy(out_bytes->data(), extrabuf.data(), use_size);
		}

		return true;
	}

	// 序列化影响行数
	bool SerializeAffectedRows(MYSQL &mysql, ByteArray *out_bytes)
	{
		assert(out_bytes != nullptr);
		if (out_bytes == nullptr)
		{
			return false;
		}

		out_bytes->clear();
		std::array<char, 128> extrabuf = { '2', '\0', '1', '\0', 'A', 'f', 'f', 'e', 'c', 't', 'e', 'd', 'R', 'o', 'w', 's', '\0' };
		size_t use_size = strlen(extrabuf.data());
		my_ulonglong rows = mysql_affected_rows(&mysql);
		use_size += snprintf(extrabuf.data() + use_size, extrabuf.size() - use_size, "%lld", rows) + 1;
		out_bytes->resize(use_size);
		memcpy(out_bytes->data(), extrabuf.data(), use_size);
		return true;
	}

	// 序列化影响行数和插入id
	bool SerializeAffectedRowsAndInsertID(MYSQL &mysql, ByteArray *out_bytes)
	{
		assert(out_bytes != nullptr);
		if (out_bytes == nullptr)
		{
			return false;
		}

		out_bytes->clear();
		std::array<char, 128> extrabuf = { '2', '\0', '2', '\0', 'A', 'f', 'f', 'e', 'c', 't', 'e', 'd', 'R', 'o', 'w', 's',
			'\0', 'I', 'n', 's', 'e', 'r', 't', 'I', 'D' };
		size_t use_size = strlen(extrabuf.data());
		my_ulonglong id = mysql_insert_id(&mysql);
		my_ulonglong rows = mysql_affected_rows(&mysql);
		use_size += snprintf(extrabuf.data() + use_size, extrabuf.size() - use_size, "%lld", id) + 1;
		use_size += snprintf(extrabuf.data() + use_size, extrabuf.size() - use_size, "%lld", rows) + 1;
		out_bytes->resize(use_size);
		memcpy(out_bytes->data(), extrabuf.data(), use_size);
		return true;
	}
}

/************************************************************************/
/************************************************************************/

ConnectorMySQL::ConnectorMySQL(const std::string &host, unsigned short port, const std::string &user, const std::string &passwd)
	: InterfaceConnector(host, port, user, passwd)
	, connected_(false)
{
	Connect();
}

ConnectorMySQL::ConnectorMySQL(const std::string &host, unsigned short port, const std::string &user, const std::string &passwd, int timeout)
	:InterfaceConnector(host, port, user, passwd, timeout)
	, connected_(false)
{
	Connect(timeout);
}

ConnectorMySQL::~ConnectorMySQL()
{
	Disconnect();
}

const char* ConnectorMySQL::Name() const
{
	return select_db_.c_str();
}

void ConnectorMySQL::Connect()
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
		mysql_set_server_option(&mysql_, MYSQL_OPTION_MULTI_STATEMENTS_ON);
	}
}

void ConnectorMySQL::Connect(int timeout)
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
		mysql_set_server_option(&mysql_, MYSQL_OPTION_MULTI_STATEMENTS_ON);
	}
}

void ConnectorMySQL::Disconnect()
{
	if (connected_)
	{
		mysql_close(&mysql_);
		connected_ = false;
	}
}

bool ConnectorMySQL::IsConnected() const
{
	return connected_;
}

void ConnectorMySQL::SelectDatabase(const char *db, ErrorCode &error_code)
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

void ConnectorMySQL::SetCharacterSet(const char *csname, ErrorCode &error_code)
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


ByteArray ConnectorMySQL::AffectedRows(const ByteArray &command, ErrorCode &error_code)
{
	if (IsConnected())
	{
		mysql_real_query(&mysql_, command.data(), command.size());
		int error = mysql_errno(&mysql_);
		if (error != 0)
		{
			error_code.SetError(error, mysql_error(&mysql_));
			return ByteArray();
		}

		ByteArray bytes;
		mysql_stuff::SerializeAffectedRows(mysql_, &bytes);
		return bytes;
	}
	else
	{
		throw NotConnected();
	}
}

ByteArray ConnectorMySQL::Call(const ByteArray &command, ErrorCode &error_code)
{
	if (IsConnected())
	{
		mysql_real_query(&mysql_, command.data(), command.size());
		int error = mysql_errno(&mysql_);
		if (error != 0)
		{
			error_code.SetError(error, mysql_error(&mysql_));
			return ByteArray();
		}

		ByteArray bytes;
		MYSQL_RES *sql_result = mysql_store_result(&mysql_);
		if (sql_result != nullptr)
		{
			mysql_stuff::Serialize(sql_result, &bytes);
			mysql_free_result(sql_result);
		}
		else
		{
			ProcedureMySQL procedure(command);
			if (procedure.HasVariable())
			{
				ByteArray query_variable = procedure.QueryVariableValue();
				mysql_real_query(&mysql_, query_variable.data(), query_variable.size());
				int error = mysql_errno(&mysql_);
				if (error != 0)
				{
					error_code.SetError(error, mysql_error(&mysql_));
					return ByteArray();
				}

				MYSQL_RES *sql_result = mysql_store_result(&mysql_);
				mysql_stuff::Serialize(sql_result, &bytes);
				mysql_free_result(sql_result);
			}
		}
		return bytes;
	}
	else
	{
		throw NotConnected();
	}
}

ByteArray ConnectorMySQL::Select(const ByteArray &command, ErrorCode &error_code)
{
	if (IsConnected())
	{
		mysql_real_query(&mysql_, command.data(), command.size());
		int error = mysql_errno(&mysql_);
		if (error != 0)
		{
			error_code.SetError(error, mysql_error(&mysql_));
			return ByteArray();
		}

		ByteArray bytes;
		MYSQL_RES *sql_result = mysql_store_result(&mysql_);
		mysql_stuff::Serialize(sql_result, &bytes);
		mysql_free_result(sql_result);
		return bytes;
	}
	else
	{
		throw NotConnected();
	}
}

ByteArray ConnectorMySQL::Insert(const ByteArray &command, ErrorCode &error_code)
{
	if (IsConnected())
	{
		mysql_real_query(&mysql_, command.data(), command.size());
		int error = mysql_errno(&mysql_);
		if (error != 0)
		{
			error_code.SetError(error, mysql_error(&mysql_));
			return ByteArray();
		}

		ByteArray bytes;
		mysql_stuff::SerializeAffectedRowsAndInsertID(mysql_, &bytes);
		return bytes;
	}
	else
	{
		throw NotConnected();
	}
}

ByteArray ConnectorMySQL::Update(const ByteArray &command, ErrorCode &error_code)
{
	return AffectedRows(command, error_code);
}

ByteArray ConnectorMySQL::Delete(const ByteArray &command, ErrorCode &error_code)
{
	return AffectedRows(command, error_code);
}
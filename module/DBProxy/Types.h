#ifndef __DBPROXY_TYPES_H__
#define __DBPROXY_TYPES_H__

#include <vector>
#include <string>
#include <stdexcept>

namespace dbproxy
{
	typedef std::vector<char> ByteArray;

	enum class ActionType
	{
		kSelect = 1,
		kInsert = 2,
		kUpdate = 3,
		kDelete = 4,
	};

	class ErrorCode
	{
	public:
		ErrorCode()
			: error_code_(0)
		{
		}

		ErrorCode(ErrorCode &&other)
		{
			error_code_ = other.error_code_;
			message_ = std::move(other.message_);
			other.error_code_ = 0;
		}

		ErrorCode& operator= (ErrorCode &&other)
		{
			error_code_ = other.error_code_;
			message_ = std::move(other.message_);
			other.error_code_ = 0;
			return *this;
		}

		int Value() const
		{
			return error_code_;
		}

		const char* Message() const
		{
			return message_.c_str();
		}

		void SetError(int error_code, const char *message)
		{
			error_code_ = error_code;
			message_ = message;
		}

		operator bool() const
		{
			return error_code_ != 0;
		}

	private:
		int error_code_;
		std::string message_;
	};

	class NotConnected : public std::logic_error
	{
	public:
		NotConnected() : std::logic_error("database not connected") {}
	};

	class ConnectionError : public std::runtime_error
	{
	public:
		ConnectionError() : std::runtime_error("database connection error") {}
	};
}

#endif
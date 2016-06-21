#ifndef __TYPES_H__
#define __TYPES_H__

#include <string>
#include <stdexcept>

namespace dbproxy
{
	enum class CommandType
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
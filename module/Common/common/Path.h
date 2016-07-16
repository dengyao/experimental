#ifndef __PATH_H__
#define __PATH_H__

#include <array>
#include <string>

class path
{
public:
	static const char sep;

	static const char* pardir;

	static std::string curdir();

	static bool isdir(const std::string &s);

	static bool isfile(const std::string &s);

	static bool exists(const std::string &s);

	static size_t getsize(const std::string &s);

	static time_t getmtime(const std::string &s);

	static bool mkdir(const std::string &s);

	static bool rmdir(const std::string &s);

	static std::array<std::string, 2> split(const std::string &s);

	static std::array<std::string, 2> splitext(const std::string &s);

	static std::string basename(const std::string &s);

	static std::string dirname(const std::string &s);
};

#endif
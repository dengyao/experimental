#ifndef __STRING_HELPER_H__
#define __STRING_HELPER_H__

#include <vector>
#include <string>
#include <limits>

namespace string_helper
{
	template <typename ...Args>
	std::string format(const char *format, Args ...args)
	{
		std::string result;
		result.resize(snprintf(nullptr, 0, format, args ...) + 1);
		snprintf(const_cast<char *>(result.data()), result.size(), format, args ...);
		return result;
	}

	std::string replace(const std::string &str, const char *old, const char *new_str);

	std::vector<std::string> split(const std::string &str, const char *sep, size_t maxsplit = std::numeric_limits<size_t>::max());

	std::vector<std::string> rsplit(const std::string &str, const char *sep, size_t maxsplit = std::numeric_limits<size_t>::max());
}

#endif

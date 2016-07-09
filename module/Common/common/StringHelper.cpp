#include "StringHelper.h"
#include <cstring>

namespace stringhlper
{
	std::vector<std::string> split(const std::string &str, const char *sep, size_t maxsplit)
	{
		size_t len = strlen(sep);
		std::vector<std::string> result;
		std::string::size_type offset = 0;
		std::string::size_type current_pos = str.find(sep);
		while (current_pos != std::string::npos && result.size() < maxsplit)
		{
			if (current_pos - offset > 0)
			{
				result.push_back(str.substr(offset, current_pos - offset));
			}
			offset = current_pos + len;
			current_pos = str.find(sep, offset);
		}
		if (offset != str.size() && result.size() < maxsplit)
		{
			result.push_back(str.substr(offset));
		}
		return result;
	}

	std::vector<std::string> rsplit(const std::string &str, const char *sep, size_t maxsplit)
	{
		size_t len = strlen(sep);
		std::vector<std::string> result;
		std::string::size_type offset = str.size();
		std::string::size_type current_pos = str.rfind(sep);
		while (current_pos != std::string::npos && result.size() < maxsplit)
		{
			if (offset - current_pos - len > 0)
			{
				result.push_back(str.substr(current_pos + len, offset - current_pos - len));
			}
			offset = current_pos;
			current_pos = str.rfind(sep, offset - 1);
		}
		if (offset != 0 && result.size() < maxsplit)
		{
			result.push_back(str.substr(0, offset));
		}
		return result;
	}
}
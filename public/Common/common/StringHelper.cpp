#include "StringHelper.h"
#include <cstring>

namespace string_helper
{
	std::string replace(const std::string &str, const char *old, const char *new_str)
	{
		size_t pos = 0;
		std::vector<size_t> keyframe;
		const int old_length = strlen(old);
		const int new_length = strlen(new_str);
		const int diff = new_length - old_length;
		while (pos != std::string::npos)
		{
			pos = str.find(old, pos);
			if (pos != std::string::npos)
			{
				keyframe.push_back(pos);
				pos += old_length;
			}
		}

		std::string new_string;
		new_string.resize(str.size() + keyframe.size() * new_length - keyframe.size() * old_length);

		if (!keyframe.empty())
		{
			size_t begin = 0;
			if (keyframe.front() != 0)
			{
				memcpy(const_cast<char*>(new_string.data()), str.data(), keyframe.front());
				begin = 1;
			}
			for (size_t i = begin; i < keyframe.size(); ++i)
			{
				const size_t old_offset = keyframe[i];
				const size_t new_offset = keyframe[i] + diff * i;
				const size_t copy_length = i == keyframe.size() - 1 ? str.size() - old_offset : keyframe[i + 1] - old_offset;
				memcpy(const_cast<char*>(new_string.data() + new_offset), new_str, new_length);
				memcpy(const_cast<char*>(new_string.data() + new_offset + new_length), str.data() + old_offset + old_length, copy_length);
			}
		}

		return new_string;
	}

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
#include "Path.h"
#include <vector>
#include <cassert>
#include <cstring>

# if defined(_WIN32) || defined(__CYGWIN__)
#   define USE_WINDOWS_API
# endif

# if defined(USE_WINDOWS_API)
#include <direct.h>
# else
#include <unistd.h>
#include <sys/stat.h>
# endif

# if defined(USE_WINDOWS_API)
const char path::sep = '\\';
# else
const char path::sep = '/';
# endif
const char* path::pardir = "..";

std::string path::curdir()
{
	std::string path;
	std::vector<char> buffer(128);
	while (true)
	{
		if (getcwd(const_cast<char*>(buffer.data()), buffer.size()) != 0)
		{
			path.resize(strlen(buffer.data()));
			memcpy(const_cast<char*>(path.data()), buffer.data(), path.size());
		}
		else if (errno == ERANGE)
		{
			buffer.resize(buffer.size() * 2);
			continue;
		}
		break;
	}
	return path;
}

bool path::isdir(const std::string &s)
{
   struct stat state;
   if (::stat(s.c_str(), &state) != 0)
   {
	   return false;
   }
   return (state.st_mode & S_IFDIR) != 0;
}

bool path::isfile(const std::string &s)
{
	struct stat state;
	if (::stat(s.c_str(), &state) != 0)
	{
		return false;
	}
	return (state.st_mode & S_IFREG) != 0;
}

bool path::exists(const std::string &s)
{
	struct stat state;
	return ::stat(s.c_str(), &state) == 0;
}

size_t path::getsize(const std::string &s)
{
	struct stat state;
	if (::stat(s.c_str(), &state) == 0)
	{
		return state.st_size;
	}
	return 0;
}

time_t path::getmtime(const std::string &s)
{
	struct stat state;
	if (::stat(s.c_str(), &state) == 0)
	{
		return state.st_mtime;
	}
	return 0;
}

std::array<std::string, 2> path::split(const std::string &s)
{
	struct stat state;
	std::array<std::string, 2> dir_and_file;
	if (::stat(s.c_str(), &state) != 0)
	{
		return dir_and_file;
	}

	if ((state.st_mode & S_IFDIR) > 0)
	{
		dir_and_file[0] = s;
	}
	else if ((state.st_mode & S_IFREG) > 0)
	{
		const size_t pos = s.rfind(sep);
		if (pos != std::string::npos)
		{
			dir_and_file[0] = s.substr(0, pos + 1);
			dir_and_file[1] = s.substr(pos + 1, s.size() - pos);
		}
		else
		{
			dir_and_file[1] = s;
		}
	}
	return dir_and_file;
}

std::array<std::string, 2> path::splitext(const std::string &s)
{
	struct stat state;
	std::array<std::string, 2> dir_and_file;
	if (::stat(s.c_str(), &state) != 0)
	{
		return dir_and_file;
	}

	if ((state.st_mode & S_IFDIR) > 0)
	{
		dir_and_file[0] = s;
	}
	else if ((state.st_mode & S_IFREG) > 0)
	{
		const size_t pos = s.rfind('.');
		assert(pos != std::string::npos);
		if (pos != std::string::npos)
		{
			dir_and_file[0] = s.substr(0, pos);
			dir_and_file[1] = s.substr(pos, s.size() - pos);
		}	
	}
	return dir_and_file;
}

std::string path::basename(const std::string &s)
{
	return split(s)[1];
}

std::string path::dirname(const std::string &s)
{
	return split(s)[0];
}
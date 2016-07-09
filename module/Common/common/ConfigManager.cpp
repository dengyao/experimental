#include "ConfigManager.h"
#include <cassert>
#include <cstring>

#if defined(_WIN32)
#include <windows.h>
typedef FILETIME FileTimeType;
#else
#include <sys/stat.h>
typedef time_t   FileTimeType;
#endif

struct ConfigInfo
{
	std::string filename;
	FileTimeType last_write_time;
	ConfigInterface* instance;

	ConfigInfo(const std::string &file, ConfigInterface *object)
		: filename(file)
		, instance(object)
	{
		memset(&last_write_time, 0, sizeof(last_write_time));
	}
};

bool GetConfigFileLastWriteTime(const std::string &filename, FileTimeType &out_last_write_time)
{
#if defined(_WIN32)
	WIN32_FIND_DATAA find_file_data;
	HANDLE handle = FindFirstFileA(filename.c_str(), &find_file_data);
	if (handle == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	out_last_write_time = find_file_data.ftLastWriteTime;
	FindClose(handle);
#else
	struct stat state;
	if (stat(filename.c_str(), &state) != 0)
	{
		return false;
	}
	out_last_write_time = state.st_mtime;
#endif
	return true;
}

void ConfigManager::Append(const std::string &filename, ConfigInterface *instance)
{
	assert(instance != nullptr);
	auto found = config_files_.find(filename);
	assert(found == config_files_.end());
	if (instance != nullptr && found == config_files_.end())
	{
		config_files_.insert(std::make_pair(filename, std::make_unique<ConfigInfo>(filename, instance)));
	}
}

void ConfigManager::Remove(const std::string &filename)
{
	auto found = config_files_.find(filename);
	assert(found != config_files_.end());
	if (found != config_files_.end())
	{
		if (found->second->instance != nullptr)
		{
			found->second->instance->UnLoad();
		}
		config_files_.erase(found);
	}
}

void ConfigManager::RemoveAll()
{
	for (auto &pair : config_files_)
	{
		pair.second->instance->UnLoad();
	}
	config_files_.clear();
}

bool ConfigManager::LoadAllConfigFiles()
{
	load_error_files_.clear();
	FileTimeType last_write_time;
	for (auto &pair : config_files_)
	{
		if (!pair.second->instance->Load(pair.first))
		{
			assert(false);
			load_error_files_.push_back(pair.first);
		}
		else
		{
			if (GetConfigFileLastWriteTime(pair.first, last_write_time))
			{
				pair.second->last_write_time = last_write_time;
			}
		}
	}
	return load_error_files_.empty();
}

bool ConfigManager::HasLoadErrorConfigFiles(const std::vector<std::string> *&out_error_files) const
{
	if (load_error_files_.empty())
	{
		out_error_files = nullptr;
		return false;
	}
	else
	{
		out_error_files = &load_error_files_;
		return true;
	}
}

std::vector<std::string> ConfigManager::GetConfigFiles() const
{
	std::vector<std::string> files;
	for (const auto &pair : config_files_)
	{
		files.push_back(pair.first);
	}
	return files;
}

void ConfigManager::CheckUpdateConfigFiles()
{
	load_error_files_.clear();
	FileTimeType last_write_time;
	for (auto &pair : config_files_)
	{
		ConfigInfoPointer &config = pair.second;
		if (config->instance != nullptr && GetConfigFileLastWriteTime(config->filename, last_write_time))
		{
			if (memcmp(&last_write_time, &config->last_write_time, sizeof(FileTimeType)) != 0)
			{
				if (config->instance->Load(config->filename))
				{
					if (GetConfigFileLastWriteTime(config->filename, last_write_time))
					{
						config->last_write_time = last_write_time;
					}
				}
				else
				{
					assert(false);
					load_error_files_.push_back(config->filename);
				}
			}
		}
	}
}
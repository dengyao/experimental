#include "ConfigManager.h"
#include <cassert>
#include <cstring>
#include "Path.h"

struct ConfigInfo
{
	std::string filename;
	time_t last_write_time;
	ConfigInterface* instance;

	ConfigInfo(const std::string &file, ConfigInterface *object)
		: filename(file)
		, instance(object)
	{
		memset(&last_write_time, 0, sizeof(last_write_time));
	}
};

bool GetConfigFileLastWriteTime(const std::string &filename, time_t &out_last_write_time)
{
	out_last_write_time = path::getmtime(filename);
	if (out_last_write_time == 0)
	{
		return false;
	}
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
	time_t last_write_time;
	load_error_files_.clear();
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
	time_t last_write_time;
	load_error_files_.clear();
	for (auto &pair : config_files_)
	{
		ConfigInfoPointer &config = pair.second;
		if (config->instance != nullptr && GetConfigFileLastWriteTime(config->filename, last_write_time))
		{
			if (config->last_write_time == last_write_time)
			{
				if (config->instance->Load(config->filename))
				{
					config->last_write_time = last_write_time;
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
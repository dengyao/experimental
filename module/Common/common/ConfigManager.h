#ifndef __CONFIG_MANAGER_H__
#define __CONFIG_MANAGER_H__

#include <vector>
#include <unordered_map>
#include "Singleton.h"
#include "ConfigInterface.h"

#if defined(_WIN32)
#include <windows.h>
typedef FILETIME FileTimeType;
#else
#include <sys/stat.h>
typedef time_t   FileTimeType;
#endif

class ConfigManager : public Singleton< ConfigManager >
{
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

public:
	ConfigManager() = default;
	~ConfigManager() = default;

public:
	void Append(const std::string &filename, ConfigInterface *instance);

	void Remove(const std::string &filename);

	void RemoveAll();

	bool LoadAllConfigFiles();

	bool HasLoadErrorConfigFiles(const std::vector<std::string> *&out_error_files) const;

	std::vector<std::string> GetConfigFiles() const;

private:
	void CheckUpdateConfigFiles();

	bool GetConfigFileLastWriteTime(const std::string &filename, FileTimeType &out_last_write_time);

private:
	std::vector<std::string> load_error_files_;
	std::unordered_map<std::string, ConfigInfo> config_files_;

};

#endif

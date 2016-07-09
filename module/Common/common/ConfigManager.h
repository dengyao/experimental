#ifndef __CONFIG_MANAGER_H__
#define __CONFIG_MANAGER_H__

#include <vector>
#include <memory>
#include <unordered_map>
#include "Singleton.h"
#include "ConfigInterface.h"

struct ConfigInfo;

class ConfigManager : public Singleton< ConfigManager >
{
	typedef std::unique_ptr<ConfigInfo> ConfigInfoPointer;

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

private:
	std::vector<std::string> load_error_files_;
	std::unordered_map<std::string, ConfigInfoPointer> config_files_;
};

#endif

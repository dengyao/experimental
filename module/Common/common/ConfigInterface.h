#ifndef __CONFIG_INTERFACE_H__
#define __CONFIG_INTERFACE_H__

#include <string>

class ConfigInterface
{
public:
	ConfigInterface() = default;
	virtual ~ConfigInterface() = default;

public:
	virtual bool Load(const std::string &filename) = 0;

	virtual void UnLoad() = 0;

protected:
	ConfigInterface(const ConfigInterface &) = delete;
	void operator=(const ConfigInterface &) = delete;
};

#endif
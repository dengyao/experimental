#ifndef __MODULE_MANAGER_H__
#define __MODULE_MANAGER_H__

#include <map>
#include <vector>
#include "Module.h"
#include "Singleton.h"

class ModuleManager : public Singleton< ModuleManager >
{
	SINGLETON(ModuleManager);

public:
	size_t ModuleCount() const;

	std::vector<Module*> ModuleList();

	Module* Moudle(const std::string &name);

	Module* AddModule(const std::string &name);

	void RemoveAllModule();

	void RemoveModule(const std::string &name);

private:
	std::map<std::string, Module> module_map_;
};

#endif

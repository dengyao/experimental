#include "ModuleManager.h"


ModuleManager::ModuleManager()
{
}

ModuleManager::~ModuleManager()
{
	RemoveAllModule();
}

size_t ModuleManager::ModuleCount() const
{
	return module_map_.size();
}

std::vector<Module*> ModuleManager::ModuleList()
{
	std::vector<Module*> modules;
	for (auto iter = module_map_.begin(); iter != module_map_.end(); ++iter)
	{
		modules.push_back(&iter->second);
	}
	return modules;
}

Module* ModuleManager::Moudle(const std::string &name)
{
	auto found = module_map_.find(name);
	if (found == module_map_.end())
	{
		return nullptr;
	}
	return &found->second;
}

Module* ModuleManager::AddModule(const std::string &name)
{
	try
	{
		Module module(name);
		module_map_.insert(std::make_pair(name, std::move(module)));
	}
	catch (ModuleLoadFail &)
	{
		return nullptr;
	}
	return Moudle(name);
}

void ModuleManager::RemoveAllModule()
{
	module_map_.clear();
}

void ModuleManager::RemoveModule(const std::string &name)
{
	auto found = module_map_.find(name);
	if (found != module_map_.end())
	{
		module_map_.erase(found);
	}
}
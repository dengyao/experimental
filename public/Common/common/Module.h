#ifndef __MODULE_H__
#define __MODULE_H__

#include <string>
#include <cstddef>
#include <stdexcept>

# ifndef _WIN32
#include <dlfcn.h>
typedef void* MoudleHandle;
# else
#include <Windows.h>
typedef HMODULE MoudleHandle;
# endif

class ModuleLoadFail : public std::runtime_error
{
public:
	ModuleLoadFail()
		: std::runtime_error("module not found!")
	{
	}
};

class Module
{
	Module(const Module&) = delete;
	Module& operator= (const Module&) = delete;

public:
	Module(const std::string &module_name)
		: module_(nullptr)
		, module_name_(module_name)
	{
		OpenModule();
	}

	Module(Module &&other)
		: module_(other.module_)
		, module_name_(std::move(other.module_name_))
	{
		other.module_ = nullptr;
	}

	~Module()
	{
		CloseModule();
	}

public:
	MoudleHandle GetModuleHandle()
	{
		return module_;
	}

	const char* GetModuleName() const
	{
		return module_name_.c_str();
	}

public:
	template <typename IModeluInterface>
	IModeluInterface* CreateInstance(const std::string &function_name) const
	{
		if (module_ == nullptr)
		{
			return nullptr;
		}
		typedef void*(*func)();
# ifndef _WIN32
		void *creator = dlsym(module_, function_name.c_str());
# else
		void *creator = GetProcAddress(module_, function_name.c_str());
# endif
		if (creator == nullptr)
		{
			return nullptr;
		}
		return reinterpret_cast<IModeluInterface*>((reinterpret_cast<func>(creator))());
	}

private:
	void OpenModule()
	{
		if (module_ == nullptr)
		{
# ifndef _WIN32
			module_ = dlopen(module_name_.c_str(), RTLD_NOW);
# else
			module_ = LoadLibrary(module_name_.c_str());
# endif
			if (module_ == nullptr)
			{
				throw ModuleLoadFail();
			}
		}
	}

	void CloseModule()
	{
		if (module_ != nullptr)
		{
# ifndef _WIN32
			dlclose(module_);
# else
			FreeLibrary(module_);
# endif
			module_ = nullptr;
		}
	}

private:
	MoudleHandle module_;
	std::string  module_name_;
};

# ifndef _WIN32
#define DLLEXPORT
# else
#define DLLEXPORT __declspec(dllexport)
# endif

#define MODULE_EXPORT_DLL(CREATE_FUNCTION_NAME, ModuleInterface)  \
    extern "C" DLLEXPORT void* CREATE_FUNCTION_NAME()             \
    {                                                             \
        return new ModuleInterface();                             \
    };

#endif
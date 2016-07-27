#include "Logging.h"
#include <atomic>
#include <common/Path.h>

namespace log_stuff
{
	class Base : public std::enable_shared_from_this<Base>
	{
	public:
		Base() = default;
		virtual ~Base() = default;
	};

	std::shared_ptr<Base> g_warp_logger;
	std::shared_ptr<spdlog::async_logger> g_logger;

	template <typename Mutex>
	class Logging : public Base
	{
	public:
		Logging(const std::string &filename)
		{
			spdlog::drop_all();
			std::vector<spdlog::sink_ptr> sinks;
# ifdef _DEBUG
			sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink<Mutex> >());
# endif
			sinks.push_back(std::make_shared<spdlog::sinks::daily_file_sink<Mutex> >(filename, "txt", 23, 59));
			g_logger = std::make_shared<spdlog::async_logger>("logger", begin(sinks), end(sinks), 8192);
# ifdef _DEBUG
			g_logger->set_level(spdlog::level::trace);
# else
			g_logger->set_level(spdlog::level::info);
# endif
			g_logger->set_pattern("%Y%m%d %H:%M:%S.%f %t %l %v");
			spdlog::register_logger(g_logger);
		}
	};
}

const std::shared_ptr<spdlog::async_logger>& logger()
{
	return log_stuff::g_logger;
}

bool OnceInitLogSettings(const std::string &out_dir, const std::string &process_name, bool use_multithread)
{
	static std::atomic_bool initialized;
	if (initialized)
	{
		assert(!initialized);
		return false;
	}

	if (!path::exists(out_dir))
	{
		if (!path::mkdir(out_dir))
		{
			return false;
		}
	}
	else if (!path::isdir(out_dir))
	{
		return false;
	}
	const std::string filename = out_dir + path::sep + process_name;

	try
	{
		initialized = true;
		if (use_multithread)
		{
			log_stuff::g_warp_logger = std::make_shared<log_stuff::Logging<std::mutex> >(filename);
		}
		else
		{
			log_stuff::g_warp_logger = std::make_shared<log_stuff::Logging<spdlog::details::null_mutex> >(filename);
		}
	}
	catch (const spdlog::spdlog_ex&)
	{
		log_stuff::g_warp_logger.reset();
		initialized = false;
		return false;
	}
	return true;
}
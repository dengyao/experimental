#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <spdlog/spdlog.h>

const std::shared_ptr<spdlog::async_logger>& logger();

bool OnceInitLogSettings(const std::string &out_dir, const std::string &process_name, bool use_multithread = false);

#endif
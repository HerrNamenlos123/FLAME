#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
	
#ifndef DEPLOY

#define LOG_SET_LOGLEVEL(...)			FLAME::logger->set_level(__VA_ARGS__)
#define INIT_LOGGER()			        {	if (!FLAME::logger) {	\
													spdlog::set_pattern("%^[%T] %n: %v%$"); \
													FLAME::logger = spdlog::stdout_color_mt("FLAME"); \
													LOG_SET_LOGLEVEL(spdlog::level::trace); \
												} \
											}

#define LOG_TRACE(...)					{ INIT_LOGGER(); FLAME::logger->trace(__VA_ARGS__);			 }
#define LOG_WARN(...)					{ INIT_LOGGER(); FLAME::logger->warn(__VA_ARGS__);				 }
#define LOG_DEBUG(...)					{ INIT_LOGGER(); FLAME::logger->debug(__VA_ARGS__);			 }
#define LOG_INFO(...)					{ INIT_LOGGER(); FLAME::logger->info(__VA_ARGS__);				 }
#define LOG_ERROR(...)					{ INIT_LOGGER(); FLAME::logger->error(__VA_ARGS__);			 }
#define LOG_CRITICAL(...)				{ INIT_LOGGER(); FLAME::logger->critical(__VA_ARGS__);			 }

#else

#define LOG_SET_LOGLEVEL(...)			{ ; }

#define LOG_TRACE(...)					{ ; }
#define LOG_WARN(...)					{ ; }
#define LOG_DEBUG(...)					{ ; }
#define LOG_INFO(...)					{ ; }
#define LOG_ERROR(...)					{ ; }
#define LOG_CRITICAL(...)				{ ; }

#endif

namespace FLAME {

	enum LogLevel {
		LOG_LEVEL_TRACE,
		LOG_LEVEL_DEBUG,
		LOG_LEVEL_INFO,
		LOG_LEVEL_WARN,
		LOG_LEVEL_ERROR,
		LOG_LEVEL_CRITICAL
	};

	extern std::shared_ptr<spdlog::logger> logger;

	/// <summary>
	/// <para>Sets the log level for the NetLib. Available:</para>
	/// <para>NetLib::LOG_LEVEL_TRACE</para>
	/// <para>NetLib::LOG_LEVEL_DEBUG</para>
	/// <para>NetLib::LOG_LEVEL_INFO</para>
	/// <para>NetLib::LOG_LEVEL_WARN</para>
	/// <para>NetLib::LOG_LEVEL_ERROR</para>
	/// <para>NetLib::LOG_LEVEL_CRITICAL</para>
	/// </summary>
	void SetLogLevel(enum LogLevel logLevel);

}
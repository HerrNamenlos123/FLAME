
#include "Log.h"

namespace FLAME {

    // ==========================
    // ===      Logging       ===
    // ==========================

    std::shared_ptr<spdlog::logger> logger;

    void SetLogLevel(enum LogLevel logLevel) {
        INIT_LOGGER();

        switch (logLevel) {
        case LOG_LEVEL_TRACE:       LOG_SET_LOGLEVEL(spdlog::level::trace);     break;
        case LOG_LEVEL_DEBUG:       LOG_SET_LOGLEVEL(spdlog::level::debug);     break;
        case LOG_LEVEL_INFO:        LOG_SET_LOGLEVEL(spdlog::level::info);      break;
        case LOG_LEVEL_WARN:        LOG_SET_LOGLEVEL(spdlog::level::warn);      break;
        case LOG_LEVEL_ERROR:       LOG_SET_LOGLEVEL(spdlog::level::err);       break;
        case LOG_LEVEL_CRITICAL:    LOG_SET_LOGLEVEL(spdlog::level::critical);  break;
        }
    }

}

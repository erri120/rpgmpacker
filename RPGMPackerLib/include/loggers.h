#pragma once

#include <spdlog/spdlog.h>

struct Loggers {
    const std::shared_ptr<spdlog::logger>& logger;
    const std::shared_ptr<spdlog::logger>& errorLogger;

    Loggers(const std::shared_ptr<spdlog::logger>& logger, const std::shared_ptr<spdlog::logger>& errorLogger)
        : logger(logger), errorLogger(errorLogger) { }
};
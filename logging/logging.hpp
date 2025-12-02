// logging.hpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <string>
#include <format>
#include <mutex>

namespace Istok::Logging {

enum class Level {
    OFF = 0,
    CRITICAL = 1,
    ERROR = 2,
    WARNING = 3,
    INFO = 4,
    DEBUG = 5,
    TRACE = 6,
};

class Logger {
public:
    virtual ~Logger() = default;

    virtual void log(Level level, std::string_view message) = 0;
};

class NoneLogger final : public Logger {
    void log(Level level, std::string_view message) override {}
};

NoneLogger none;

class LoggerRegistry {
public:
    struct Entry {
        Logger* logger;
        Level maxLevel;
    };

    static LoggerRegistry& getGlobal() {
        static LoggerRegistry instance;
        return instance;
    }

    LoggerRegistry() {
        set("", none, Level::OFF);
    }

    void set(std::string_view name, Logger& logger, Level maxLevel) {
        std::lock_guard lock(mutex_);
        loggers_[std::string(name)] = Entry{&logger, maxLevel};
    }

    Entry get(std::string_view name) {
        std::lock_guard<std::mutex> lock(mutex_);
        do {
            if (
                auto it = loggers_.find(std::string(name));
                it != loggers_.end()
            ) {
                return it->second;
            }
            
            size_t pos = name.find_last_of('.');
            if (pos == std::string_view::npos) {
                pos = 0;
            }
            name = name.substr(0, pos);
        } while (!name.empty());

        assert(false);
    }

private:
    std::unordered_map<std::string, Entry> loggers_;
    std::mutex mutex_;
};

}  // namespace Istok::Logging

#define SET_LOGGER(name, logger_ref, maxLevel) \
    Istok::Logging::LoggerRegistry::getGlobal().set(name, logger_ref, maxLevel)

#define WITH_LOGGER(name) \
    static Istok::Logging::LoggerRegistry::Entry istok_logging_get_logger = \
        []() { \
            static auto entry = \
                Istok::Logging::LoggerRegistry::getGlobal().get(name); \
            return entry; \
        }

#define WITH_THIS_LOGGER(name) \
    static Istok::Logging::LoggerRegistry::Entry istok_logging_get_logger() { \
        static auto entry = \
            Istok::Logging::LoggerRegistry::getGlobal().get(name); \
        return entry; \
    }

#define LOG_WITH_LEVEL(level, format, ...) \
    do { \
        if (level <= istok_logging_get_logger().maxLevel) { \
            istok_logging_get_logger().logger->log( \
                std::format(fmt, ##__VA_ARGS__)); \
        } \
    } while (0)

#define LOG_CRITICAL(format, ...) \
    LOG_WITH_LEVEL(Istok::Logging::Level::CRITICAL, format, ##__VA_ARGS__)

#define LOG_ERROR(format, ...) \
    LOG_WITH_LEVEL(Istok::Logging::Level::ERROR, format, ##__VA_ARGS__)

#define LOG_WARNING(format, ...) \
    LOG_WITH_LEVEL(Istok::Logging::Level::WARNING, format, ##__VA_ARGS__)

#define LOG_INFO(format, ...) \
    LOG_WITH_LEVEL(Istok::Logging::Level::INFO, format, ##__VA_ARGS__)

#define LOG_DEBUG(format, ...) \
    LOG_WITH_LEVEL(Istok::Logging::Level::DEBUG, format, ##__VA_ARGS__)

#define LOG_TRACE(format, ...) \
    LOG_WITH_LEVEL(Istok::Logging::Level::TRACE, format, ##__VA_ARGS__)

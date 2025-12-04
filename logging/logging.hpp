// logging.hpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <string>
#include <format>
#include <mutex>
#include <iostream>
#include <map>
#include <unordered_map>
#include <cassert>

namespace Istok::Logging {

enum class Level {
    off = 0,
    critical = 10,
    error = 20,
    warning = 30,
    info = 40,
    debug = 50,
    trace = 60,
    all = 100,
};


const static std::map<Level, std::string> logLevelStr = {
    {Level::off, "OFF"},
    {Level::critical, "CRITICAL"},
    {Level::error, "ERROR"},
    {Level::warning, "WARNING"},
    {Level::info, "INFO"},
    {Level::debug, "DEBUG"},
    {Level::trace, "TRACE"},
    {Level::all, "ALL"},
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


class TerminalLogger final : public Logger {
    void log(Level level, std::string_view message) override {
        std::cout << logLevelStr.at(level) << ": " << message << std::endl;
    }
};

TerminalLogger terminal;

class LoggerRegistry {
public:
    struct Entry {
        Logger* logger;
        Level maxLevel;
    };

    static LoggerRegistry& getGlobalInstance() {
        static LoggerRegistry instance;
        return instance;
    }

    LoggerRegistry() {
        set("", none, Level::off);
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

            assert(!name.empty());
            size_t pos = name.find_last_of('.');
            if (pos == std::string_view::npos) {
                pos = 0;
            }
            name = name.substr(0, pos);
        } while (true);
    }

private:
    std::unordered_map<std::string, Entry> loggers_;
    std::mutex mutex_;
};

}  // namespace Istok::Logging

#define SET_LOGGER(name, logger_ref, maxLevel) \
    ::Istok::Logging:: \
        LoggerRegistry::getGlobalInstance().set(name, logger_ref, maxLevel)

#define WITH_LOGGER(name) \
    static auto istok_logging_get_logger = \
        []() { \
            static auto entry = ::Istok::Logging \
                ::LoggerRegistry::getGlobalInstance().get(name); \
            return entry; \
        }

#define CLASS_WITH_LOGGER(name) \
    static ::Istok::Logging::LoggerRegistry::Entry \
    istok_logging_get_logger() { \
        static auto entry = \
            ::Istok::Logging::LoggerRegistry::getGlobalInstance().get(name); \
        return entry; \
    }

#define LOG_WITH_LEVEL(level, fmt, ...) \
    do { \
        ::Istok::Logging::LoggerRegistry::Entry entry = \
            istok_logging_get_logger(); \
        if (level <= entry.maxLevel) { \
            entry.logger->log(level, std::format(fmt, ##__VA_ARGS__)); \
        } \
    } while (0)

#define LOG_CRITICAL(format, ...) \
    LOG_WITH_LEVEL(::Istok::Logging::Level::critical, format, ##__VA_ARGS__)

#define LOG_ERROR(format, ...) \
    LOG_WITH_LEVEL(::Istok::Logging::Level::error, format, ##__VA_ARGS__)

#define LOG_WARNING(format, ...) \
    LOG_WITH_LEVEL(::Istok::Logging::Level::warning, format, ##__VA_ARGS__)

#define LOG_INFO(format, ...) \
    LOG_WITH_LEVEL(::Istok::Logging::Level::info, format, ##__VA_ARGS__)

#define LOG_DEBUG(format, ...) \
    LOG_WITH_LEVEL(::Istok::Logging::Level::debug, format, ##__VA_ARGS__)

#define LOG_TRACE(format, ...) \
    LOG_WITH_LEVEL(::Istok::Logging::Level::trace, format, ##__VA_ARGS__)

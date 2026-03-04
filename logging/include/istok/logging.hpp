// logging.hpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <format>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>

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


inline const static std::map<Level, std::string> logLevelStr = {
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
public:
    static NoneLogger& GetInstance();
    void log(Level level, std::string_view message) override {}

private:
    NoneLogger() = default;
};


class TerminalLogger final : public Logger {
public:
    static TerminalLogger& GetInstance();
    void log(Level level, std::string_view message) override;

private:
    TerminalLogger() = default;
};


class LoggerRegistry {
public:
    struct Entry {
        Logger* logger;
        Level maxLevel;
    };

    static LoggerRegistry& getGlobalInstance();

    LoggerRegistry();

    void set(std::string_view name, Logger& logger, Level maxLevel);
    Entry get(std::string_view name);

private:
    std::unordered_map<std::string, Entry> loggers_;
    std::mutex mutex_;
};

template<typename... Args>
std::string format(
    std::string_view prefix,
    std::format_string<Args...> fmt,
    Args&&... args
) {
    std::string result;
    result.reserve(prefix.size() + 128);
    result = std::move(prefix);
    std::format_to(
        std::back_inserter(result),
        fmt, std::forward<Args>(args)...);
    return result;
}

}  // namespace Istok::Logging

#define SET_LOGGER(name, logger_ref, maxLevel) \
    ::Istok::Logging:: \
        LoggerRegistry::getGlobalInstance().set(name, logger_ref, maxLevel)

#define SET_LOGOFF(name) \
    SET_LOGGER(name, ::Istok::Logging::NoneLogger::GetInstance(), \
        ::Istok::Logging::Level::off)

#define SET_LOGTERM_CRITICAL(name) \
    SET_LOGGER(name, ::Istok::Logging::TerminalLogger::GetInstance(), \
        ::Istok::Logging::Level::critical)

#define SET_LOGTERM_ERROR(name) \
    SET_LOGGER(name, ::Istok::Logging::TerminalLogger::GetInstance(), \
        ::Istok::Logging::Level::error)

#define SET_LOGTERM_WARNING(name) \
    SET_LOGGER(name, ::Istok::Logging::TerminalLogger::GetInstance(), \
        ::Istok::Logging::Level::warning)

#define SET_LOGTERM_INFO(name) \
    SET_LOGGER(name, ::Istok::Logging::TerminalLogger::GetInstance(), \
        ::Istok::Logging::Level::info)

#define SET_LOGTERM_DEBUG(name) \
    SET_LOGGER(name, ::Istok::Logging::TerminalLogger::GetInstance(), \
        ::Istok::Logging::Level::debug)

#define SET_LOGTERM_TRACE(name) \
    SET_LOGGER(name, ::Istok::Logging::TerminalLogger::GetInstance(), \
        ::Istok::Logging::Level::trace)

#define SET_LOGTERM_ALL(name) \
    SET_LOGGER(name, ::Istok::Logging::TerminalLogger::GetInstance(), \
        ::Istok::Logging::Level::all)

#define WITH_LOGGER_PREFIX(name, prefix) \
    static auto istok_logging_get_logger = \
        []() { \
            static auto entry = ::Istok::Logging \
                ::LoggerRegistry::getGlobalInstance().get(name); \
            return entry; }; \
    static auto istok_logging_get_prefix = \
        []() -> ::std::string& { \
            static ::std::string value = prefix; \
            return value; }; \

#define WITH_LOGGER(name) WITH_LOGGER_PREFIX(name, "") \

#define CLASS_WITH_LOGGER_PREFIX(name, prefix) \
    static ::Istok::Logging::LoggerRegistry::Entry \
    istok_logging_get_logger() { \
        static auto entry = \
            ::Istok::Logging::LoggerRegistry::getGlobalInstance().get(name); \
        return entry; }; \
    static ::std::string& \
    istok_logging_get_prefix() { \
        static ::std::string value = prefix; \
        return value; }

#define CLASS_WITH_LOGGER(name) CLASS_WITH_LOGGER_PREFIX(name, "")

#define LOG_WITH_LEVEL(level, fmt, ...) \
    do { \
        ::Istok::Logging::LoggerRegistry::Entry entry = \
            istok_logging_get_logger(); \
        if (level <= entry.maxLevel) { \
            entry.logger->log( \
                level, ::Istok::Logging::format( \
                    istok_logging_get_prefix(), fmt, ##__VA_ARGS__)); \
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

// logging.hpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "logging.hpp"

#include <iostream>
#include <cassert>

namespace Istok::Logging {

// class NoneLogger

//static
NoneLogger& NoneLogger::GetInstance() {
    static NoneLogger instance;
    return instance;
}


// class TerminalLogger

//static
TerminalLogger& TerminalLogger::GetInstance() {
    static TerminalLogger instance;
    return instance;
}

void TerminalLogger::log(Level level, std::string_view message) {
    std::cout << "[" << logLevelStr.at(level) << "] "
        << message << std::endl;
}


// class LoggerRegistry

// static
LoggerRegistry& LoggerRegistry::getGlobalInstance() {
    static LoggerRegistry instance;
    return instance;
}

LoggerRegistry::LoggerRegistry() {
    set("", NoneLogger::GetInstance(), Level::off);
}

void LoggerRegistry::set(
    std::string_view name, Logger& logger, Level maxLevel
) {
    std::lock_guard lock(mutex_);
    loggers_[std::string(name)] =
        Entry{&logger, maxLevel};
}

LoggerRegistry::Entry LoggerRegistry::get(std::string_view name) {
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

}  // namespace Istok::Logging

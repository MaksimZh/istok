// logging.hpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <string>

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

class Logger {};

void set(std::string name, Logger& logger, Level maxLevel) {}

}  // namespace Istok::Logging

#define WITH_LOGGER(name)  // TODO
#define LOG_ERROR(...)  // TODO

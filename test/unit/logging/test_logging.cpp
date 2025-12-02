// test_logging.cpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <logging.hpp>

#include <string>
#include <format>

using namespace Istok::Logging;

namespace {
    struct MockLogger : public Logger {};
}

TEST_CASE("Logging - WITH_LOGGER", "[unit][logging]") {
    MockLogger logger1;
    MockLogger logger2;
    MockLogger logger3;
    SET_LOGGER("_test1", logger1, Level::OFF);
    SET_LOGGER("_test2", logger2, Level::WARNING);
    SET_LOGGER("_test3", logger3, Level::TRACE);
    [](){
        WITH_LOGGER("_test1");
        int i = 1;
        LOG_CRITICAL("critical {}", i);
        LOG_ERROR("error {}", i);
        LOG_WARNING("warning {}", i);
        LOG_INFO("info {}", i);
        LOG_DEBUG("debug {}", i);
        LOG_TRACE("trace {}", i);
    }();
    [](){
        WITH_LOGGER("_test2");
        int i = 2;
        LOG_CRITICAL("critical {}", i);
        LOG_ERROR("error {}", i);
        LOG_WARNING("warning {}", i);
        LOG_INFO("info {}", i);
        LOG_DEBUG("debug {}", i);
        LOG_TRACE("trace {}", i);
    }();
    [](){
        WITH_LOGGER("_test3");
        int i = 3;
        LOG_CRITICAL("critical {}", i);
        LOG_ERROR("error {}", i);
        LOG_WARNING("warning {}", i);
        LOG_INFO("info {}", i);
        LOG_DEBUG("debug {}", i);
        LOG_TRACE("trace {}", i);
    }();
}

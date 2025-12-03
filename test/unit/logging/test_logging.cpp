// test_logging.cpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <logging.hpp>

#include <vector>
#include <string>
#include <format>

using namespace Istok::Logging;

namespace {
    struct MockLogger : public Logger {
        std::vector<std::string> entries;

        void log(Level level, std::string_view message) override {
            entries.push_back(std::format("{}: {}", static_cast<int>(level), message));
        }
    };
}

TEST_CASE("Logging - WITH_LOGGER", "[unit][logging]") {
    MockLogger logger1;
    MockLogger logger2;
    MockLogger logger3;
    SET_LOGGER("_test_logging_WITH_LOGGER_1", logger1, Level::OFF);
    SET_LOGGER("_test_logging_WITH_LOGGER_2", logger2, Level::WARNING);
    SET_LOGGER("_test_logging_WITH_LOGGER_3", logger3, Level::TRACE);
    [](){
        WITH_LOGGER("_test_logging_WITH_LOGGER_1");
        int i = 1;
        LOG_CRITICAL("critical {}", i);
        LOG_ERROR("error {}", i);
        LOG_WARNING("warning {}", i);
        LOG_INFO("info {}", i);
        LOG_DEBUG("debug {}", i);
        LOG_TRACE("trace {}", i);
    }();
    [](){
        WITH_LOGGER("_test_logging_WITH_LOGGER_2");
        int i = 2;
        LOG_CRITICAL("critical {}", i);
        LOG_ERROR("error {}", i);
        LOG_WARNING("warning {}", i);
        LOG_INFO("info {}", i);
        LOG_DEBUG("debug {}", i);
        LOG_TRACE("trace {}", i);
    }();
    [](){
        WITH_LOGGER("_test_logging_WITH_LOGGER_3");
        int i = 3;
        LOG_CRITICAL("critical {}", i);
        LOG_ERROR("error {}", i);
        LOG_WARNING("warning {}", i);
        LOG_INFO("info {}", i);
        LOG_DEBUG("debug {}", i);
        LOG_TRACE("trace {}", i);
    }();
    REQUIRE(logger1.entries == std::vector<std::string>{});
    REQUIRE(logger2.entries == std::vector<std::string>{
        "1: critical 2",
        "2: error 2",
        "3: warning 2",
    });
    REQUIRE(logger3.entries == std::vector<std::string>{
        "1: critical 3",
        "2: error 3",
        "3: warning 3",
        "4: info 3",
        "5: debug 3",
        "6: trace 3",
    });
}

TEST_CASE("Logging - CLASS_WITH_LOGGER", "[unit][logging]") {
    MockLogger logger1;
    MockLogger logger2;
    MockLogger logger3;
    SET_LOGGER("_test_logging_CLASS_WITH_LOGGER_1", logger1, Level::OFF);
    SET_LOGGER("_test_logging_CLASS_WITH_LOGGER_2", logger2, Level::WARNING);
    SET_LOGGER("_test_logging_CLASS_WITH_LOGGER_3", logger3, Level::TRACE);
    
    class C1 {
    public:
        static void foo() {
            int i = 1;
            LOG_CRITICAL("static critical {}", i);
            LOG_ERROR("static error {}", i);
            LOG_WARNING("static warning {}", i);
            LOG_INFO("static info {}", i);
            LOG_DEBUG("static debug {}", i);
            LOG_TRACE("static trace {}", i);
        }
        
        void boo() {
            int i = 1;
            LOG_CRITICAL("critical {}", i);
            LOG_ERROR("error {}", i);
            LOG_WARNING("warning {}", i);
            LOG_INFO("info {}", i);
            LOG_DEBUG("debug {}", i);
            LOG_TRACE("trace {}", i);
        }
    
    private:
        CLASS_WITH_LOGGER("_test_logging_CLASS_WITH_LOGGER_1");
    };

    class C2 {
    public:
        static void foo() {
            int i = 2;
            LOG_CRITICAL("static critical {}", i);
            LOG_ERROR("static error {}", i);
            LOG_WARNING("static warning {}", i);
            LOG_INFO("static info {}", i);
            LOG_DEBUG("static debug {}", i);
            LOG_TRACE("static trace {}", i);
        }
        
        void boo() {
            int i = 2;
            LOG_CRITICAL("critical {}", i);
            LOG_ERROR("error {}", i);
            LOG_WARNING("warning {}", i);
            LOG_INFO("info {}", i);
            LOG_DEBUG("debug {}", i);
            LOG_TRACE("trace {}", i);
        }
    
    private:
        CLASS_WITH_LOGGER("_test_logging_CLASS_WITH_LOGGER_2");
    };

    class C3 {
    public:
        static void foo() {
            int i = 3;
            LOG_CRITICAL("static critical {}", i);
            LOG_ERROR("static error {}", i);
            LOG_WARNING("static warning {}", i);
            LOG_INFO("static info {}", i);
            LOG_DEBUG("static debug {}", i);
            LOG_TRACE("static trace {}", i);
        }
        
        void boo() {
            int i = 3;
            LOG_CRITICAL("critical {}", i);
            LOG_ERROR("error {}", i);
            LOG_WARNING("warning {}", i);
            LOG_INFO("info {}", i);
            LOG_DEBUG("debug {}", i);
            LOG_TRACE("trace {}", i);
        }
    
    private:
        CLASS_WITH_LOGGER("_test_logging_CLASS_WITH_LOGGER_3");
    };

    C1::foo();
    C1().boo();
    C2::foo();
    C2().boo();
    C3::foo();
    C3().boo();

    REQUIRE(logger1.entries == std::vector<std::string>{});
    REQUIRE(logger2.entries == std::vector<std::string>{
        "1: static critical 2",
        "2: static error 2",
        "3: static warning 2",
        "1: critical 2",
        "2: error 2",
        "3: warning 2",
    });
    REQUIRE(logger3.entries == std::vector<std::string>{
        "1: static critical 3",
        "2: static error 3",
        "3: static warning 3",
        "4: static info 3",
        "5: static debug 3",
        "6: static trace 3",
        "1: critical 3",
        "2: error 3",
        "3: warning 3",
        "4: info 3",
        "5: debug 3",
        "6: trace 3",
    });
}

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

TEST_CASE("Logging", "[unit][logging]") {
    MockLogger logger;
    set("unittest", logger, Level::INFO);
    WITH_LOGGER("unittest");
    LOG_ERROR("foo");
}

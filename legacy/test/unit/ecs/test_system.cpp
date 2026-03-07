// test_manager.cpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include "helper.hpp"

#include <ecs/system.hpp>

#include <string>
#include <format>
#include <memory>

using namespace Istok::ECS;


namespace {

class MockSystem: public System {
public:
    MockSystem(std::string id, std::vector<std::string>& log)
    : id_(id), log_(log) {
        log_.push_back(std::format("create {}", id_));
    }

    ~MockSystem() {
        log_.push_back(std::format("destroy {}", id_));
    }
    
    void run() override {
        log_.push_back(std::format("run {}", id_));
    }

private:
    std::string id_;
    std::vector<std::string>& log_;
};

} // namespace


TEST_CASE("ECS - system stack", "[unit][ecs]") {
    std::vector<std::string> log;

    SECTION("no systems") {
        SystemStack stack;
        REQUIRE(stack.empty() == true);
        stack.run();
    }

    SECTION("run single system") {
        {
            SystemStack stack;
            stack.push(std::make_unique<MockSystem>("A", log));
            REQUIRE(log.back() == "create A");
            REQUIRE(stack.empty() == false);
            stack.run();
            REQUIRE(log.back() == "run A");
        }
        REQUIRE(log == std::vector<std::string>{
            "create A",
            "run A",
            "destroy A",
        });
    }

    SECTION("pop single system") {
        {
            SystemStack stack;
            stack.push(std::make_unique<MockSystem>("A", log));
            REQUIRE(log.back() == "create A");
            REQUIRE(stack.empty() == false);
            stack.pop();
            REQUIRE(log.back() == "destroy A");
            REQUIRE(stack.empty() == true);
        }
        REQUIRE(log == std::vector<std::string>{
            "create A",
            "destroy A",
        });
    }

    SECTION("run multiple systems") {
        {
            SystemStack stack;
            stack.push(std::make_unique<MockSystem>("A", log));
            REQUIRE(log.back() == "create A");
            stack.push(std::make_unique<MockSystem>("B", log));
            REQUIRE(log.back() == "create B");
            stack.push(std::make_unique<MockSystem>("C", log));
            REQUIRE(log.back() == "create C");
            REQUIRE(stack.empty() == false);
            stack.run();
            REQUIRE(log == std::vector<std::string>{
                "create A",
                "create B",
                "create C",
                "run A",
                "run B",
                "run C",
            });
        }
        REQUIRE(log == std::vector<std::string>{
            "create A",
            "create B",
            "create C",
            "run A",
            "run B",
            "run C",
            "destroy C",
            "destroy B",
            "destroy A",
        });
    }

    SECTION("pop and run multiple systems") {
        {
            SystemStack stack;
            stack.push(std::make_unique<MockSystem>("A", log));
            stack.run();
            stack.push(std::make_unique<MockSystem>("B", log));
            stack.run();
            stack.push(std::make_unique<MockSystem>("C", log));
            stack.run();
            stack.pop();
            stack.run();
            stack.pop();
            stack.run();
            stack.pop();
            stack.run();
        }
        REQUIRE(log == std::vector<std::string>{
            "create A",
            "run A",
            "create B",
            "run A",
            "run B",
            "create C",
            "run A",
            "run B",
            "run C",
            "destroy C",
            "run A",
            "run B",
            "destroy B",
            "run A",
            "destroy A",
        });
    }

    SECTION("clear") {
        SystemStack stack;
        stack.push(std::make_unique<MockSystem>("A", log));
        stack.push(std::make_unique<MockSystem>("B", log));
        stack.push(std::make_unique<MockSystem>("C", log));
        stack.clear();
        REQUIRE(log == std::vector<std::string>{
            "create A",
            "create B",
            "create C",
            "destroy C",
            "destroy B",
            "destroy A",
        });
    }
}

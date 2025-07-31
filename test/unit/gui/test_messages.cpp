// test_messages.cpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/core/messages.hpp>

using namespace Istok::GUI;

#include <memory>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

TEST_CASE("GUI messages - simple queue", "[unit][gui]") {
    SimpleQueue<std::unique_ptr<int>> queue;
    REQUIRE(queue.empty() == true);

    SECTION("push and take") {
        auto a = std::make_unique<int>(42);
        queue.push(std::move(a));
        REQUIRE(queue.empty() == false);
        auto b = queue.take();
        REQUIRE(b != nullptr);
        REQUIRE(*b == 42);
        REQUIRE(queue.empty() == true);
    }

    SECTION("multiple items") {
        auto a = std::make_unique<int>(0);
        auto b = std::make_unique<int>(1);
        auto c = std::make_unique<int>(2);
        queue.push(std::move(a));
        queue.push(std::move(b));
        queue.push(std::move(c));
        auto a1 = queue.take();
        auto b1 = queue.take();
        auto c1 = queue.take();
        REQUIRE(queue.empty() == true);
        REQUIRE(*a1 == 0);
        REQUIRE(*b1 == 1);
        REQUIRE(*c1 == 2);
    }

    SECTION("mixed scenario") {
        queue.push(std::make_unique<int>(0));
        queue.push(std::make_unique<int>(1));
        REQUIRE(*queue.take() == 0);
        queue.push(std::make_unique<int>(2));
        queue.push(std::make_unique<int>(3));
        REQUIRE(*queue.take() == 1);
        REQUIRE(*queue.take() == 2);
        queue.push(std::make_unique<int>(4));
        REQUIRE(*queue.take() == 3);
        REQUIRE(*queue.take() == 4);
        REQUIRE(queue.empty() == true);
    }
}


TEST_CASE("GUI messages - synchronized queue", "[unit][gui]") {
    SyncQueue<int> queue;
    REQUIRE(queue.empty() == true);

    SECTION("linear usage") {
        queue.push(0);
        queue.push(1);
        REQUIRE(queue.take() == 0);
        queue.push(2);
        queue.push(3);
        REQUIRE(queue.take() == 1);
        REQUIRE(queue.take() == 2);
        queue.push(4);
        REQUIRE(queue.take() == 3);
        REQUIRE(queue.take() == 4);
        REQUIRE(queue.empty() == true);
    }

    SECTION("muti-thread usage") {
        std::thread second([&queue]{
            for (int i = 0; i < 20; ++i) {
                queue.push(std::move(i));
                std::this_thread::sleep_for(1ms);
            }
        });
        for (int i = 0; i < 20; ++i) {
            REQUIRE(queue.take() == i);
            std::this_thread::sleep_for(1ms);
        }
        second.join();
    }
}

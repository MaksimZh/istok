// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <tools/exchange.hpp>

#include <memory>

using namespace Istok::Tools;

TEST_CASE("Tools - queue", "[unit][tools]") {
    Queue<int> queue;
    REQUIRE(queue.empty() == true);

    SECTION("push and take") {
        queue.push(42);
        REQUIRE(queue.empty() == false);
        REQUIRE(queue.take() == 42);
        REQUIRE(queue.empty() == true);
    }

    SECTION("multiple items") {
        queue.push(0);
        queue.push(1);
        queue.push(2);
        REQUIRE(queue.empty() == false);
        REQUIRE(queue.take() == 0);
        REQUIRE(queue.take() == 1);
        REQUIRE(queue.empty() == false);
        REQUIRE(queue.take() == 2);
        REQUIRE(queue.empty() == true);
    }

    SECTION("mixed scenario") {
        queue.push(0);
        queue.push(1);
        REQUIRE(queue.take() == 0);
        queue.push(2);
        queue.push(3);
        REQUIRE(queue.take() == 1);
        REQUIRE(queue.take() == 2);
        REQUIRE(queue.empty() == false);
        queue.push(4);
        REQUIRE(queue.take() == 3);
        REQUIRE(queue.empty() == false);
        REQUIRE(queue.take() == 4);
        REQUIRE(queue.empty() == true);
    }
}


TEST_CASE("Tools - queue - moving", "[unit][tools]") {
    Queue<std::unique_ptr<int>> queue;
    REQUIRE(queue.empty() == true);

    auto a = std::make_unique<int>(42);
    queue.push(std::move(a));
    REQUIRE(queue.empty() == false);
    auto b = queue.take();
    REQUIRE(b != nullptr);
    REQUIRE(*b == 42);
    REQUIRE(queue.empty() == true);
}

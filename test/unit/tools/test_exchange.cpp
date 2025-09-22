// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <tools/exchange.hpp>

#include <memory>

using namespace Istok::Tools;

TEST_CASE("Tools - queue", "[unit][tools]") {
    Queue<int> queue;
    
    SECTION("empty") {
        REQUIRE(queue.take() == std::nullopt);
    }

    SECTION("push and take") {
        queue.push(42);
        REQUIRE(queue.take() == 42);
        REQUIRE(queue.take() == std::nullopt);
    }

    SECTION("multiple items") {
        queue.push(0);
        queue.push(1);
        queue.push(2);
        REQUIRE(queue.take() == 0);
        REQUIRE(queue.take() == 1);
        REQUIRE(queue.take() == 2);
        REQUIRE(queue.take() == std::nullopt);
    }

    SECTION("mixed scenario") {
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
        REQUIRE(queue.take() == std::nullopt);
    }
}


TEST_CASE("Tools - returning dispatcher", "[unit][tools]") {
    ReturningDispatcher<int, int> chain;
    
    SECTION("empty") {
        REQUIRE(chain(1) == std::nullopt);
    }

    SECTION("single") {
        chain.chainProcessor([](int x) {
            return (x % 2 == 0) ? std::optional<int>(x * 10) : std::nullopt;
        });
        REQUIRE(chain(1) == std::nullopt);
        REQUIRE(chain(2) == 20);
    }

    SECTION("multi") {
        chain.chainProcessor([](int x) {
            return (x % 2 == 0) ? std::optional<int>(x * 10) : std::nullopt;
        });
        chain.chainProcessor([](int x) {
            return (x % 3 == 0) ? std::optional<int>(x * 100) : std::nullopt;
        });
        chain.chainProcessor([](int x) {
            return (x % 5 == 0) ? std::optional<int>(x * 1000) : std::nullopt;
        });
        REQUIRE(chain(1) == std::nullopt);
        REQUIRE(chain(2) == 20);
        REQUIRE(chain(3) == 300);
        REQUIRE(chain(4) == 40);
        REQUIRE(chain(5) == 5000);
        REQUIRE(chain(6) == 60);
        REQUIRE(chain(7) == std::nullopt);
    }
}


TEST_CASE("Tools - message bus", "[unit][tools]") {
    MessageBus<int> bus;

    SECTION("subscription") {
        bus.push(1);
        int a = 0;
        bus.subscribe([&a](const int& x) { a = x; });
        REQUIRE(a == 0);
        bus.push(2);
        REQUIRE(a == 2);
        int b = 0;
        bus.subscribe([&b](const int& x) { b = x; });
        REQUIRE(b == 0);
        bus.push(3);
        REQUIRE(a == 3);
        REQUIRE(b == 3);
    }

    SECTION("inner messaging") {
        std::vector<int> a;
        bus.subscribe([&a](const int& x) { a.push_back(x); });
        bus.subscribe([&bus](const int& x) {
            if (x > 0) {
                bus.push(x - 1);
            }
        });
        bus.push(3);
        REQUIRE(a == std::vector<int>{3, 2, 1, 0});
    }
}

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


TEST_CASE("Tools - consuming dispatcher", "[unit][tools]") {
    ConsumingDispatcher<int> dispatcher;
    
    SECTION("empty") {
        REQUIRE(dispatcher(1) == 1);
    }

    SECTION("single") {
        int a = 0;
        dispatcher.chainConsumer([&a](int&& x) {
            if (x % 2 != 0) {
                return std::optional<int>(x);
            }
            a = x;
            return std::optional<int>();
        });
        REQUIRE(dispatcher(1) == 1);
        REQUIRE(a == 0);
        REQUIRE(dispatcher(2) == std::nullopt);
        REQUIRE(a == 2);
    }

    SECTION("multi") {
        int a = 0;
        dispatcher.chainConsumer([&a](int&& x) {
            if (x % 2 != 0) {
                return std::optional<int>(x);
            }
            a = x;
            return std::optional<int>();
        });
        int b = 0;
        dispatcher.chainConsumer([&b](int&& x) {
            if (x % 3 != 0) {
                return std::optional<int>(x);
            }
            b = x;
            return std::optional<int>();
        });
        int c = 0;
        dispatcher.chainConsumer([&c](int&& x) {
            if (x % 5 != 0) {
                return std::optional<int>(x);
            }
            c = x;
            return std::optional<int>();
        });
        REQUIRE(dispatcher(1) == 1);
        REQUIRE(a == 0);
        REQUIRE(b == 0);
        REQUIRE(c == 0);
        REQUIRE(dispatcher(2) == std::nullopt);
        REQUIRE(a == 2);
        REQUIRE(b == 0);
        REQUIRE(c == 0);
        REQUIRE(dispatcher(3) == std::nullopt);
        REQUIRE(a == 2);
        REQUIRE(b == 3);
        REQUIRE(c == 0);
        REQUIRE(dispatcher(4) == std::nullopt);
        REQUIRE(a == 4);
        REQUIRE(b == 3);
        REQUIRE(c == 0);
        REQUIRE(dispatcher(5) == std::nullopt);
        REQUIRE(a == 4);
        REQUIRE(b == 3);
        REQUIRE(c == 5);
        REQUIRE(dispatcher(6) == std::nullopt);
        REQUIRE(a == 6);
        REQUIRE(b == 3);
        REQUIRE(c == 5);
        REQUIRE(dispatcher(7) == 7);
        REQUIRE(a == 6);
        REQUIRE(b == 3);
        REQUIRE(c == 5);
    }
}


TEST_CASE("Tools - processor chain", "[unit][tools]") {
    ProcessorChain<int, int> dispatcher;
    
    SECTION("empty") {
        REQUIRE(dispatcher(1) == std::nullopt);
    }

    SECTION("single") {
        dispatcher.chainProcessor([](int x) {
            return (x % 2 == 0) ? std::optional<int>(x * 10) : std::nullopt;
        });
        REQUIRE(dispatcher(1) == std::nullopt);
        REQUIRE(dispatcher(2) == 20);
    }

    SECTION("multi") {
        dispatcher.chainProcessor([](int x) {
            return (x % 2 == 0) ? std::optional<int>(x * 10) : std::nullopt;
        });
        dispatcher.chainProcessor([](int x) {
            return (x % 3 == 0) ? std::optional<int>(x * 100) : std::nullopt;
        });
        dispatcher.chainProcessor([](int x) {
            return (x % 5 == 0) ? std::optional<int>(x * 1000) : std::nullopt;
        });
        REQUIRE(dispatcher(1) == std::nullopt);
        REQUIRE(dispatcher(2) == 20);
        REQUIRE(dispatcher(3) == 300);
        REQUIRE(dispatcher(4) == 40);
        REQUIRE(dispatcher(5) == 5000);
        REQUIRE(dispatcher(6) == 60);
        REQUIRE(dispatcher(7) == std::nullopt);
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

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


TEST_CASE("Tools - acceptor chain", "[unit][tools]") {
    AcceptorChain<int> chain;

    SECTION("empty") {
        chain(1);
    }

    SECTION("single") {
        int a = 0;
        chain.chainAcceptor([&a](int x) { a = x; });
        chain(1);
        REQUIRE(a == 1);
    }

    SECTION("multi") {
        int a = 0;
        chain.chainAcceptor([&a](int x) { a = x; });
        int b = 0;
        chain.chainAcceptor([&b](int x) { b = x; });
        int c = 0;
        chain.chainAcceptor([&c](int x) { c = x; });
        chain(1);
        REQUIRE(a == 1);
        REQUIRE(b == 1);
        REQUIRE(c == 1);
    }
}


TEST_CASE("Tools - consumer chain", "[unit][tools]") {
    ConsumerChain<int> chain;
    
    SECTION("empty") {
        REQUIRE(chain.dispatch(1) == 1);
    }

    SECTION("single") {
        int a = 0;
        chain.chainConsumer([&a](int&& x) {
            if (x % 2 != 0) {
                return std::optional<int>(x);
            }
            a = x;
            return std::optional<int>();
        });
        REQUIRE(chain.dispatch(1) == 1);
        REQUIRE(a == 0);
        REQUIRE(chain.dispatch(2) == std::nullopt);
        REQUIRE(a == 2);
    }

    SECTION("multi") {
        int a = 0;
        chain.chainConsumer([&a](int&& x) {
            if (x % 2 != 0) {
                return std::optional<int>(x);
            }
            a = x;
            return std::optional<int>();
        });
        int b = 0;
        chain.chainConsumer([&b](int&& x) {
            if (x % 3 != 0) {
                return std::optional<int>(x);
            }
            b = x;
            return std::optional<int>();
        });
        int c = 0;
        chain.chainConsumer([&c](int&& x) {
            if (x % 5 != 0) {
                return std::optional<int>(x);
            }
            c = x;
            return std::optional<int>();
        });
        REQUIRE(chain.dispatch(1) == 1);
        REQUIRE(a == 0);
        REQUIRE(b == 0);
        REQUIRE(c == 0);
        REQUIRE(chain.dispatch(2) == std::nullopt);
        REQUIRE(a == 2);
        REQUIRE(b == 0);
        REQUIRE(c == 0);
        REQUIRE(chain.dispatch(3) == std::nullopt);
        REQUIRE(a == 2);
        REQUIRE(b == 3);
        REQUIRE(c == 0);
        REQUIRE(chain.dispatch(4) == std::nullopt);
        REQUIRE(a == 4);
        REQUIRE(b == 3);
        REQUIRE(c == 0);
        REQUIRE(chain.dispatch(5) == std::nullopt);
        REQUIRE(a == 4);
        REQUIRE(b == 3);
        REQUIRE(c == 5);
        REQUIRE(chain.dispatch(6) == std::nullopt);
        REQUIRE(a == 6);
        REQUIRE(b == 3);
        REQUIRE(c == 5);
        REQUIRE(chain.dispatch(7) == 7);
        REQUIRE(a == 6);
        REQUIRE(b == 3);
        REQUIRE(c == 5);
    }
}


TEST_CASE("Tools - processor chain", "[unit][tools]") {
    ProcessorChain<int, int> chain;
    
    SECTION("empty") {
        REQUIRE(chain.dispatch(1) == std::nullopt);
    }

    SECTION("single") {
        chain.chainProcessor([](int x) {
            return (x % 2 == 0) ? std::optional<int>(x * 10) : std::nullopt;
        });
        REQUIRE(chain.dispatch(1) == std::nullopt);
        REQUIRE(chain.dispatch(2) == 20);
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
        REQUIRE(chain.dispatch(1) == std::nullopt);
        REQUIRE(chain.dispatch(2) == 20);
        REQUIRE(chain.dispatch(3) == 300);
        REQUIRE(chain.dispatch(4) == 40);
        REQUIRE(chain.dispatch(5) == 5000);
        REQUIRE(chain.dispatch(6) == 60);
        REQUIRE(chain.dispatch(7) == std::nullopt);
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

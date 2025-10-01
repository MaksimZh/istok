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
    std::vector<int> log;

    SECTION("broadcasting") {
        bus.addSubscriber([&](int&& x) {
            log.push_back(100 + x);
            return x;
        });
        bus.addSubscriber([&](int&& x) {
            log.push_back(200 + x);
            return x;
        });
        bus.addSubscriber([&](int&& x) {
            log.push_back(300 + x);
            return x;
        });
        REQUIRE(log == std::vector<int>{});
        bus.push(1);
        REQUIRE(log == std::vector<int>{101, 201, 301});
        bus.push(2);
        REQUIRE(log == std::vector<int>{101, 201, 301, 102, 202, 302});
    }

    SECTION("consuming") {
        bus.addSubscriber([&](int&& x) -> std::optional<int> {
            if (x < 30) {
                return x;
            }
            log.push_back(100 + x);
            return std::nullopt;
        });
        bus.addSubscriber([&](int&& x) -> std::optional<int> {
            if (x < 20) {
                return x;
            }
            log.push_back(200 + x);
            return std::nullopt;
        });
        bus.addSubscriber([&](int&& x) -> std::optional<int> {
            if (x < 10) {
                return x;
            }
            log.push_back(300 + x);
            return std::nullopt;
        });
        REQUIRE(log == std::vector<int>{});
        bus.push(1);
        REQUIRE(log == std::vector<int>{});
        bus.push(11);
        REQUIRE(log == std::vector<int>{311});
        bus.push(21);
        REQUIRE(log == std::vector<int>{311, 221});
        bus.push(31);
        REQUIRE(log == std::vector<int>{311, 221, 131});
    }

    SECTION("inner messaging") {
        bus.addSubscriber([&](int&& x) -> std::optional<int> {
            if (x <= 0) {
                return x;
            }
            log.push_back(100 + x);
            bus.push(x - 1);
            return std::nullopt;
        });
        REQUIRE(log == std::vector<int>{});
        bus.push(4);
        REQUIRE(log == std::vector<int>{104, 103, 102, 101});
    }

    SECTION("inner exchange") {
        bus.addSubscriber([&](int&& x) -> std::optional<int> {
            if (x <= 0) {
                return x;
            }
            log.push_back(100 + x);
            bus.push(x - 10);
            if (x >= 20) {
                return x;
            }
            return std::nullopt;
        });
        bus.addSubscriber([&](int&& x) -> std::optional<int> {
            if (x <= 0) {
                return x;
            }
            log.push_back(200 + x);
            bus.push(x - 11);
            return std::nullopt;
        });
        REQUIRE(log == std::vector<int>{});
        bus.push(39);
        REQUIRE(log == std::vector<int>{
            139, 239, // send: 29, 28
            129, 229, // send: 19, 18
            128, 228, // send: 18, 17
            119,      // send: 9
            118,      // send: 8
            118,      // send: 8
            117,      // send: 7
            109, 108, 108, 107
        });
    }
}

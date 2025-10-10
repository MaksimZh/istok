// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <tools/exchange.hpp>

#include <memory>

using namespace Istok::Tools;


TEST_CASE("Tools - HandlerResult void", "[unit][tools]") {
    SECTION("default") {
        HandlerResult<int> r;
        REQUIRE(r.complete() == true);
        REQUIRE_THROWS(r.argument());
        REQUIRE_NOTHROW(r.result());
    }

    SECTION("arg") {
        HandlerResult<int> r(100);
        REQUIRE(r.complete() == false);
        REQUIRE(r.argument() == 100);
        REQUIRE_THROWS(r.result());
    }
}


TEST_CASE("Tools - HandlerResult result", "[unit][tools]") {
    SECTION("arg") {
        auto r = HandlerResult<int, int>::fromArgument(100);
        REQUIRE(r.complete() == false);
        REQUIRE(r.argument() == 100);
        REQUIRE_THROWS(r.result());
    }

    SECTION("result") {
        auto r = HandlerResult<int, int>::fromResult(100);
        REQUIRE(r.complete() == true);
        REQUIRE_THROWS(r.argument());
        REQUIRE(r.result() == 100);
    }
}


TEST_CASE("Tools - HandlerChain void", "[unit][tools]") {
    HandlerChain<int> chain;
    using Result = HandlerResult<int>;

    SECTION("empty") {
        REQUIRE(chain(1) == Result(1));
    }

    SECTION("single") {
        int a;
        chain.append([&](int x) {
            a = x;
            return (x % 2 == 0) ? Result() : Result(std::move(x));
        });
        
        a = 0;
        REQUIRE(chain(1) == Result(1));
        REQUIRE(a == 1);
        
        a = 0;
        REQUIRE(chain(2) == Result());
        REQUIRE(a == 2);
    }

    SECTION("multi") {
        int a = 0;
        chain.append([&](int x) {
            a = x;
            return (x % 2 == 0) ? Result() : Result(std::move(x));
        });
        int b = 0;
        chain.append([&](int x) {
            b = x;
            return (x % 3 == 0) ? Result() : Result(std::move(x));
        });
        int c = 0;
        chain.append([&](int x) {
            c = x;
            return (x % 5 == 0) ? Result() : Result(std::move(x));
        });

        a = b = c = 0;
        REQUIRE(chain(1) == Result(1));
        REQUIRE(a == 1);
        REQUIRE(b == 1);
        REQUIRE(c == 1);

        a = b = c = 0;
        REQUIRE(chain(2) == Result());
        REQUIRE(a == 2);
        REQUIRE(b == 0);
        REQUIRE(c == 0);

        a = b = c = 0;
        REQUIRE(chain(3) == Result());
        REQUIRE(a == 3);
        REQUIRE(b == 3);
        REQUIRE(c == 0);

        a = b = c = 0;
        REQUIRE(chain(5) == Result());
        REQUIRE(a == 5);
        REQUIRE(b == 5);
        REQUIRE(c == 5);

        a = b = c = 0;
        REQUIRE(chain(6) == Result());
        REQUIRE(a == 6);
        REQUIRE(b == 0);
        REQUIRE(c == 0);

        a = b = c = 0;
        REQUIRE(chain(7) == Result(7));
        REQUIRE(a == 7);
        REQUIRE(b == 7);
        REQUIRE(c == 7);
    }
}


TEST_CASE("Tools - HandlerChain result", "[unit][tools]") {
    HandlerChain<int, int> chain;
    using Result = HandlerResult<int, int>;

    SECTION("empty") {
        REQUIRE(chain(1) == Result::fromArgument(1));
    }

    SECTION("single") {
        int a;
        chain.append([&](int x) {
            a = x;
            return (x % 2 == 0)
                ? Result::fromResult(200 + x)
                : Result::fromArgument(std::move(x));
        });
        
        a = 0;
        REQUIRE(chain(1) == Result::fromArgument(1));
        REQUIRE(a == 1);
        
        a = 0;
        REQUIRE(chain(2) == Result::fromResult(202));
        REQUIRE(a == 2);
    }

    SECTION("multi") {
        int a = 0;
        chain.append([&](int x) {
            a = x;
            return (x % 2 == 0)
                ? Result::fromResult(200 + x)
                : Result::fromArgument(std::move(x));
        });
        int b = 0;
        chain.append([&](int x) {
            b = x;
            return (x % 3 == 0)
                ? Result::fromResult(300 + x)
                : Result::fromArgument(std::move(x));
        });
        int c = 0;
        chain.append([&](int x) {
            c = x;
            return (x % 5 == 0)
                ? Result::fromResult(500 + x)
                : Result::fromArgument(std::move(x));
        });

        a = b = c = 0;
        REQUIRE(chain(1) == Result::fromArgument(1));
        REQUIRE(a == 1);
        REQUIRE(b == 1);
        REQUIRE(c == 1);

        a = b = c = 0;
        REQUIRE(chain(2) == Result::fromResult(202));
        REQUIRE(a == 2);
        REQUIRE(b == 0);
        REQUIRE(c == 0);

        a = b = c = 0;
        REQUIRE(chain(3) == Result::fromResult(303));
        REQUIRE(a == 3);
        REQUIRE(b == 3);
        REQUIRE(c == 0);

        a = b = c = 0;
        REQUIRE(chain(5) == Result::fromResult(505));
        REQUIRE(a == 5);
        REQUIRE(b == 5);
        REQUIRE(c == 5);

        a = b = c = 0;
        REQUIRE(chain(6) == Result::fromResult(206));
        REQUIRE(a == 6);
        REQUIRE(b == 0);
        REQUIRE(c == 0);

        a = b = c = 0;
        REQUIRE(chain(7) == Result::fromArgument(7));
        REQUIRE(a == 7);
        REQUIRE(b == 7);
        REQUIRE(c == 7);
    }
}


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


TEST_CASE("Tools - message bus", "[unit][tools]") {
    MessageBus<int> bus;
    std::vector<int> log;
    using Result = HandlerResult<int>;

    SECTION("broadcasting") {
        bus.addSubscriber([&](int&& x) {
            log.push_back(100 + x);
            return Result(std::move(x));
        });
        bus.addSubscriber([&](int&& x) {
            log.push_back(200 + x);
            return Result(std::move(x));
        });
        bus.addSubscriber([&](int&& x) {
            log.push_back(300 + x);
            return Result(std::move(x));
        });
        REQUIRE(log == std::vector<int>{});
        bus.push(1);
        REQUIRE(log == std::vector<int>{101, 201, 301});
        bus.push(2);
        REQUIRE(log == std::vector<int>{101, 201, 301, 102, 202, 302});
    }

    SECTION("consuming") {
        bus.addSubscriber([&](int&& x) {
            if (x < 30) {
                return Result(std::move(x));
            }
            log.push_back(100 + x);
            return Result();
        });
        bus.addSubscriber([&](int&& x) {
            if (x < 20) {
                return Result(std::move(x));
            }
            log.push_back(200 + x);
            return Result();
        });
        bus.addSubscriber([&](int&& x) {
            if (x < 10) {
                return Result(std::move(x));
            }
            log.push_back(300 + x);
            return Result();
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
        bus.addSubscriber([&](int&& x) {
            if (x <= 0) {
                return Result(std::move(x));
            }
            log.push_back(100 + x);
            bus.push(x - 1);
            return Result();
        });
        REQUIRE(log == std::vector<int>{});
        bus.push(4);
        REQUIRE(log == std::vector<int>{104, 103, 102, 101});
    }

    SECTION("inner exchange") {
        bus.addSubscriber([&](int&& x) {
            if (x <= 0) {
                return Result(std::move(x));
            }
            log.push_back(100 + x);
            bus.push(x - 10);
            if (x >= 20) {
                return Result(std::move(x));
            }
            return Result();
        });
        bus.addSubscriber([&](int&& x) {
            if (x <= 0) {
                return Result(std::move(x));
            }
            log.push_back(200 + x);
            bus.push(x - 11);
            return Result();
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

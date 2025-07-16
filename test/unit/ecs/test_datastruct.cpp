// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <ecs/datastruct.hpp>

namespace ecs = Istok::ECS;

namespace {
    struct A {
        int value;

        bool operator==(const A&) const = default;

        struct Hasher {
            size_t operator()(const A& obj) const {
                return std::hash<int>()(obj.value);
            }
        };
    };

    struct B {
        int value;

        bool operator==(const B&) const = default;

        struct Hasher {
            size_t operator()(const B& obj) const {
                return std::hash<int>()(obj.value);
            }
        };
    };
}


TEST_CASE("ECS Data Structures - Queue", "[unit][ecs]") {
    ecs::Queue<A> queue;
    REQUIRE(queue.empty() == true);
    
    SECTION("push lvalue") {
        A value(1);
        queue.push(value);
        REQUIRE(queue.empty() == false);
        REQUIRE(queue.front() == A(1));
    }

    SECTION("push rvalue") {
        queue.push(A(1));
        REQUIRE(queue.empty() == false);
        REQUIRE(queue.front() == A(1));
    }

    SECTION("push-pop one") {
        queue.push(A(1));
        queue.pop();
        REQUIRE(queue.empty() == true);
    }

    SECTION("push-pop multi") {
        queue.push(A(1));
        queue.push(A(2));
        queue.push(A(3));
        REQUIRE(queue.empty() == false);
        REQUIRE(queue.front() == A(1));
        queue.pop();
        REQUIRE(queue.empty() == false);
        REQUIRE(queue.front() == A(2));
        queue.pop();
        REQUIRE(queue.empty() == false);
        REQUIRE(queue.front() == A(3));
        queue.pop();
        REQUIRE(queue.empty() == true);
    }

    SECTION("push-pop mix") {
        queue.push(A(1));
        queue.push(A(2));
        REQUIRE(queue.front() == A(1));
        queue.pop();
        REQUIRE(queue.front() == A(2));
        queue.push(A(3));
        queue.push(A(4));
        queue.pop();
        REQUIRE(queue.front() == A(3));
        queue.pop();
        REQUIRE(queue.front() == A(4));
        queue.pop();
        REQUIRE(queue.empty() == true);
    }
}

// test_datastruct.cpp
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


static_assert(std::ranges::forward_range<ecs::DenseRange<A>>);

TEST_CASE("ECS Data Structures - DenseArray", "[unit][ecs]") {
    ecs::DenseArray<A> array;

    REQUIRE(array.size() == 0);
    REQUIRE(std::ranges::equal(array.byElement(), std::vector<A>{}));

    SECTION("push_back lvalue") {
        A value(1);
        array.push_back(value);
        REQUIRE(array.size() == 1);
        REQUIRE(array[0] == A(1));
    }

    SECTION("single element") {
        array.push_back(A(1));

        REQUIRE(array.size() == 1);
        REQUIRE(array[0] == A(1));
        REQUIRE(std::ranges::equal(array.byElement(), std::vector{A(1)}));

        SECTION("erase") {
            array.erase(0);
            REQUIRE(array.size() == 0);
        }
    }

    SECTION("multiple elements") {
        array.push_back(A(1));
        array.push_back(A(2));
        array.push_back(A(3));
        array.push_back(A(4));

        REQUIRE(array.size() == 4);
        REQUIRE(array[0] == A(1));
        REQUIRE(array[1] == A(2));
        REQUIRE(array[2] == A(3));
        REQUIRE(array[3] == A(4));
        REQUIRE(std::ranges::equal(array.byElement(), std::vector{
            A(1), A(2), A(3), A(4)}));

        SECTION("erase last") {
            array.erase(3);
            REQUIRE(std::ranges::equal(array.byElement(), std::vector{
                A(1), A(2), A(3)}));
        }

        SECTION("erase middle") {
            array.erase(1);
            REQUIRE(array.size() == 3);
            REQUIRE(std::ranges::equal(array.byElement(), std::vector{
                A(1), A(4), A(3)}));
        }

        SECTION("erase many") {
            array.erase(1);
            array.erase(2);
            REQUIRE(std::ranges::equal(array.byElement(), std::vector{
                A(1), A(4)}));
            array.erase(0);
            REQUIRE(std::ranges::equal(array.byElement(), std::vector{A(4)}));
            array.erase(0);
            REQUIRE(array.size() == 0);
        }
    }
}


TEST_CASE("ECS Data Structures - DenseArrayPair", "[unit][ecs]") {
    ecs::DenseArrayPair<A, B> array;

    REQUIRE(array.size() == 0);

    SECTION("push_back lvalue") {
        A value1(1);
        B value2(10);
        array.push_back(value1, value2);
        REQUIRE(array.size() == 1);
        REQUIRE(array.first(0) == A(1));
        REQUIRE(array.second(0) == B(10));
    }

    SECTION("push_back rvalue") {
        array.push_back(A(1), B(10));
        REQUIRE(array.size() == 1);
        REQUIRE(array.first(0) == A(1));
        REQUIRE(array.second(0) == B(10));
    }

    SECTION("multiple elements") {
        array.push_back(A(1), B(10));
        array.push_back(A(2), B(20));
        array.push_back(A(3), B(30));
        REQUIRE(array.size() == 3);
        REQUIRE(array.first(0) == A(1));
        REQUIRE(array.first(1) == A(2));
        REQUIRE(array.first(2) == A(3));
        REQUIRE(array.second(0) == B(10));
        REQUIRE(array.second(1) == B(20));
        REQUIRE(array.second(2) == B(30));
    }

    SECTION("erase last") {
        array.push_back(A(1), B(10));
        array.push_back(A(2), B(20));
        array.push_back(A(3), B(30));
        array.erase(2);
        REQUIRE(array.size() == 2);
        REQUIRE(array.first(0) == A(1));
        REQUIRE(array.first(1) == A(2));
        REQUIRE(array.second(0) == B(10));
        REQUIRE(array.second(1) == B(20));
    }

    SECTION("erase middle") {
        array.push_back(A(1), B(10));
        array.push_back(A(2), B(20));
        array.push_back(A(3), B(30));
        array.push_back(A(4), B(40));
        array.erase(1);
        REQUIRE(array.size() == 3);
        REQUIRE(array.first(0) == A(1));
        REQUIRE(array.first(1) == A(4));
        REQUIRE(array.first(2) == A(3));
        REQUIRE(array.second(0) == B(10));
        REQUIRE(array.second(1) == B(40));
        REQUIRE(array.second(2) == B(30));
    }

    SECTION("byElement") {
        std::vector<A> elements1 = {A(1), A(2), A(3)};
        std::vector<B> elements2 = {B(10), B(20), B(30)};
        for (int i = 0; i < elements1.size(); i++) {
            array.push_back(elements1[i], elements2[i]);
        }
        REQUIRE(std::ranges::equal(array.firstElements(), elements1));
        REQUIRE(std::ranges::equal(array.secondElements(), elements2));
    }
}


TEST_CASE("ECS Data Structures - IndexMap", "[unit][ecs]") {
    ecs::IndexMap<A, A::Hasher> map;

    REQUIRE(map.contains(A(0)) == false);

    SECTION("insert lvalue") {
        A value(1);
        map.insert(value, 10);
        REQUIRE(map.contains(A(1)) == true);
        REQUIRE(map.get(A(1)) == 10);
    }

    SECTION("insert rvalue") {
        map.insert(A(1), 10);
        REQUIRE(map.contains(A(1)) == true);
        REQUIRE(map.get(A(1)) == 10);
    }

    SECTION("multiple elements") {
        map.insert(A(1), 10);
        map.insert(A(2), 20);
        map.insert(A(3), 30);
        REQUIRE(map.contains(A(0)) == false);
        REQUIRE(map.contains(A(1)) == true);
        REQUIRE(map.contains(A(2)) == true);
        REQUIRE(map.contains(A(3)) == true);
        REQUIRE(map.get(A(1)) == 10);
        REQUIRE(map.get(A(2)) == 20);
        REQUIRE(map.get(A(3)) == 30);
    }

    SECTION("erase") {
        map.insert(A(1), 10);
        map.insert(A(2), 20);
        map.insert(A(3), 30);
        map.erase(A(2));
        REQUIRE(map.contains(A(1)) == true);
        REQUIRE(map.contains(A(2)) == false);
        REQUIRE(map.contains(A(3)) == true);
        REQUIRE(map.get(A(1)) == 10);
        REQUIRE(map.get(A(3)) == 30);
    }
}

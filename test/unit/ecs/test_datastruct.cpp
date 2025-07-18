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

    SECTION("single element") {
        array.pushBack(A(1));

        REQUIRE(array.size() == 1);
        REQUIRE(array[0] == A(1));
        REQUIRE(std::ranges::equal(array.byElement(), std::vector{A(1)}));

        SECTION("replace") {
            array[0] = A(42);
            REQUIRE(std::ranges::equal(array.byElement(), std::vector{A(42)}));
        }

        SECTION("erase") {
            array.erase(0);
            REQUIRE(array.size() == 0);
        }
    }

    SECTION("pushBack lvalue") {
        A value(1);
        array.pushBack(value);
        REQUIRE(std::ranges::equal(array.byElement(), std::vector{A(1)}));
    }

    SECTION("multiple elements") {
        array.pushBack(A(1));
        array.pushBack(A(2));
        array.pushBack(A(3));
        array.pushBack(A(4));

        REQUIRE(array.size() == 4);
        REQUIRE(array[0] == A(1));
        REQUIRE(array[1] == A(2));
        REQUIRE(array[2] == A(3));
        REQUIRE(array[3] == A(4));
        REQUIRE(std::ranges::equal(array.byElement(), std::vector{
            A(1), A(2), A(3), A(4)}));

        SECTION("replace") {
            array[2] = A(42);
            REQUIRE(std::ranges::equal(array.byElement(), std::vector{
                A(1), A(2), A(42), A(4)}));
        }

        SECTION("erase last") {
            array.erase(3);
            REQUIRE(std::ranges::equal(array.byElement(), std::vector{
                A(1), A(2), A(3)}));
        }

        SECTION("erase middle") {
            array.erase(1);
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
    REQUIRE(std::ranges::equal(array.firstElements(), std::vector<A>{}));
    REQUIRE(std::ranges::equal(array.secondElements(), std::vector<B>{}));

    SECTION("single element") {
        array.pushBack(A(1), B(10));

        REQUIRE(array.size() == 1);
        REQUIRE(array.first(0) == A(1));
        REQUIRE(array.second(0) == B(10));
        REQUIRE(std::ranges::equal(array.firstElements(), std::vector{A(1)}));
        REQUIRE(std::ranges::equal(array.secondElements(), std::vector{B(10)}));

        SECTION("replace") {
            array.set(0, A(42), B(17));
            REQUIRE(std::ranges::equal(array.firstElements(), std::vector{A(42)}));
            REQUIRE(std::ranges::equal(array.secondElements(), std::vector{B(17)}));
        }

        SECTION("erase") {
            array.erase(0);
            REQUIRE(array.size() == 0);
        }
    }

    SECTION("pushBack lvalue") {
        A value1(1);
        B value2(10);
        array.pushBack(value1, value2);
        REQUIRE(std::ranges::equal(array.firstElements(), std::vector{A(1)}));
        REQUIRE(std::ranges::equal(array.secondElements(), std::vector{B(10)}));
    }

    SECTION("multiple elements") {
        array.pushBack(A(1), B(10));
        array.pushBack(A(2), B(20));
        array.pushBack(A(3), B(30));
        array.pushBack(A(4), B(40));

        REQUIRE(array.size() == 4);
        REQUIRE(array.first(0) == A(1));
        REQUIRE(array.first(1) == A(2));
        REQUIRE(array.first(2) == A(3));
        REQUIRE(array.first(3) == A(4));
        REQUIRE(std::ranges::equal(array.firstElements(), std::vector{
            A(1), A(2), A(3), A(4)}));
        REQUIRE(array.second(1) == B(20));
        REQUIRE(array.second(0) == B(10));
        REQUIRE(array.second(2) == B(30));
        REQUIRE(array.second(3) == B(40));
        REQUIRE(std::ranges::equal(array.secondElements(), std::vector{
            B(10), B(20), B(30), B(40)}));

        SECTION("replace") {
            array.set(2, A(42), B(17));
            REQUIRE(std::ranges::equal(array.firstElements(), std::vector{
                A(1), A(2), A(42), A(4)}));
            REQUIRE(std::ranges::equal(array.secondElements(), std::vector{
                B(10), B(20), B(17), B(40)}));
        }

        SECTION("erase last") {
            array.erase(3);
            REQUIRE(std::ranges::equal(array.firstElements(), std::vector{
                A(1), A(2), A(3)}));
            REQUIRE(std::ranges::equal(array.secondElements(), std::vector{
                B(10), B(20), B(30)}));
        }

        SECTION("erase middle") {
            array.erase(1);
            REQUIRE(std::ranges::equal(array.firstElements(), std::vector{
                A(1), A(4), A(3)}));
            REQUIRE(std::ranges::equal(array.secondElements(), std::vector{
                B(10), B(40), B(30)}));
        }

        SECTION("erase many") {
            array.erase(1);
            array.erase(2);
            REQUIRE(std::ranges::equal(array.firstElements(), std::vector{
                A(1), A(4)}));
            REQUIRE(std::ranges::equal(array.secondElements(), std::vector{
                B(10), B(40)}));
            array.erase(0);
            REQUIRE(std::ranges::equal(array.firstElements(), std::vector{A(4)}));
            REQUIRE(std::ranges::equal(array.secondElements(), std::vector{B(40)}));
            array.erase(0);
            REQUIRE(array.size() == 0);
        }
    }
}


TEST_CASE("ECS Data Structures - IndexMap", "[unit][ecs]") {
    ecs::IndexMap<A, A::Hasher> map;

    REQUIRE(map.contains(A(0)) == false);

    SECTION("single value") {
        map.insert(A(1), 10);
        REQUIRE(map.contains(A(1)) == true);
        REQUIRE(map.get(A(1)) == 10);

        SECTION("replace") {
            map.insert(A(1), 42);
            REQUIRE(map.contains(A(1)) == true);
            REQUIRE(map.get(A(1)) == 42);
        }

        SECTION("erase") {
            map.erase(A(1));
            REQUIRE(map.contains(A(1)) == false);
        }
    }

    SECTION("insert lvalue") {
        A value(1);
        map.insert(value, 10);
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

        SECTION("replace") {
            map.insert(A(2), 42);
            REQUIRE(map.contains(A(0)) == false);
            REQUIRE(map.contains(A(1)) == true);
            REQUIRE(map.contains(A(2)) == true);
            REQUIRE(map.contains(A(3)) == true);
            REQUIRE(map.get(A(1)) == 10);
            REQUIRE(map.get(A(2)) == 42);
            REQUIRE(map.get(A(3)) == 30);
        }

        SECTION("erase") {
            map.erase(A(2));
            REQUIRE(map.contains(A(1)) == true);
            REQUIRE(map.contains(A(2)) == false);
            REQUIRE(map.contains(A(3)) == true);
            REQUIRE(map.get(A(1)) == 10);
            REQUIRE(map.get(A(3)) == 30);
        }

        SECTION("many") {
            map.erase(A(1));
            map.erase(A(3));
            REQUIRE(map.contains(A(1)) == false);
            REQUIRE(map.contains(A(2)) == true);
            REQUIRE(map.contains(A(3)) == false);
            REQUIRE(map.get(A(2)) == 20);
            map.erase(A(2));
            REQUIRE(map.contains(A(2)) == false);
        }
    }
}


TEST_CASE("ECS Data Structures - DenseMap", "[unit][ecs]") {
    ecs::DenseMap<A, B, A::Hasher> map;

    REQUIRE(map.contains(A(0)) == false);
    REQUIRE(std::ranges::equal(map.byKey(), std::vector<A>{}));

    SECTION("single value") {
        map.insert(A(1), B(10));
        REQUIRE(map.contains(A(1)) == true);
        REQUIRE(std::ranges::equal(map.byKey(), std::vector{A(1)}));
        REQUIRE(map.get(A(1)) == B(10));

        SECTION("replace") {
            map.insert(A(1), B(42));
            REQUIRE(std::ranges::equal(map.byKey(), std::vector{A(1)}));
            REQUIRE(map.get(A(1)) == B(42));
        }

        SECTION("erase") {
            map.erase(A(1));
            REQUIRE(map.contains(A(1)) == false);
            REQUIRE(std::ranges::equal(map.byKey(), std::vector<A>{}));
        }
    }

    SECTION("insert lvalue") {
        A key(1);
        B value(10);
        map.insert(key, value);
        REQUIRE(std::ranges::equal(map.byKey(), std::vector{A(1)}));
        REQUIRE(map.get(A(1)) == B(10));
    }

    SECTION("multiple elements") {
        map.insert(A(1), B(10));
        map.insert(A(2), B(20));
        map.insert(A(3), B(30));
        map.insert(A(4), B(40));
        REQUIRE(map.contains(A(0)) == false);
        REQUIRE(map.contains(A(1)) == true);
        REQUIRE(map.contains(A(2)) == true);
        REQUIRE(map.contains(A(3)) == true);
        REQUIRE(map.contains(A(4)) == true);
        REQUIRE(std::ranges::equal(map.byKey(), std::vector{
            A(1), A(2), A(3), A(4)}));
        REQUIRE(map.get(A(1)) == B(10));
        REQUIRE(map.get(A(2)) == B(20));
        REQUIRE(map.get(A(3)) == B(30));
        REQUIRE(map.get(A(4)) == B(40));

        SECTION("replace") {
            map.insert(A(3), B(42));
            REQUIRE(std::ranges::equal(map.byKey(), std::vector{
                A(1), A(2), A(3), A(4)}));
            REQUIRE(map.get(A(1)) == B(10));
            REQUIRE(map.get(A(2)) == B(20));
            REQUIRE(map.get(A(3)) == B(42));
            REQUIRE(map.get(A(4)) == B(40));
        }

        SECTION("erase last") {
            map.erase(A(4));
            REQUIRE(std::ranges::equal(map.byKey(), std::vector{
                A(1), A(2), A(3)}));
            REQUIRE(map.get(A(1)) == B(10));
            REQUIRE(map.get(A(2)) == B(20));
            REQUIRE(map.get(A(3)) == B(30));
        }

        SECTION("erase middle") {
            map.erase(A(2));
            REQUIRE(std::ranges::equal(map.byKey(), std::vector{
                A(1), A(4), A(3)}));
            REQUIRE(map.get(A(1)) == B(10));
            REQUIRE(map.get(A(3)) == B(30));
            REQUIRE(map.get(A(4)) == B(40));
        }

        SECTION("erase many") {
            map.erase(A(2));
            map.erase(A(3));
            REQUIRE(std::ranges::equal(map.byKey(), std::vector{
                A(1), A(4)}));
            REQUIRE(map.get(A(1)) == B(10));
            REQUIRE(map.get(A(4)) == B(40));
            map.erase(A(1));
            REQUIRE(std::ranges::equal(map.byKey(), std::vector{A(4)}));
            REQUIRE(map.get(A(4)) == B(40));
            map.erase(A(4));
            REQUIRE(std::ranges::equal(map.byKey(), std::vector<A>{}));
        }
    }
}


TEST_CASE("ECS Data Structures - LimitedCounter", "[unit][ecs]") {
    ecs::LimitedCounter counter(3);

    REQUIRE(counter.get() == 0);
    REQUIRE(counter.full() == false);

    SECTION("increment") {
        counter.inc();
        REQUIRE(counter.get() == 1);
        REQUIRE(counter.full() == false);
    }

    SECTION("make full") {
        counter.inc();
        counter.inc();
        counter.inc();
        REQUIRE(counter.get() == 3);
        REQUIRE(counter.full() == true);
    }

    SECTION("extend") {
        counter.extend(2);
        counter.inc();
        counter.inc();
        counter.inc();
        REQUIRE(counter.get() == 3);
        REQUIRE(counter.full() == false);
        counter.inc();
        counter.inc();
        REQUIRE(counter.get() == 5);
        REQUIRE(counter.full() == true);
    }

    SECTION("extend full") {
        counter.inc();
        counter.inc();
        counter.inc();
        REQUIRE(counter.get() == 3);
        REQUIRE(counter.full() == true);
        counter.extend(2);
        REQUIRE(counter.get() == 3);
        REQUIRE(counter.full() == false);
        counter.inc();
        counter.inc();
        REQUIRE(counter.get() == 5);
        REQUIRE(counter.full() == true);
    }
}

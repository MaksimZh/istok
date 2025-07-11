// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <ecs/entity.hpp>


TEST_CASE("Entity - limited counter", "[unit][ecs]") {
    LimitedCounter c(0, 2);
    REQUIRE(c == 0);
    REQUIRE(c.isFull() == false);
    ++c;
    REQUIRE(c == 1);
    REQUIRE(c.isFull() == false);
    c++;
    REQUIRE(c == 2);
    REQUIRE(c.isFull() == true);
    c.extendBy(3);
    for (int i = 2; i < 5; i++) {
        REQUIRE(c == i);
        REQUIRE(c.isFull() == false);
        c++;
    }
    REQUIRE(c == 5);
    REQUIRE(c.isFull() == true);
}


TEST_CASE("Entity - index queue", "[unit][ecs]") {
    Queue<int> q;
    REQUIRE(q.empty() == true);
    q.push(1);
    REQUIRE(q.empty() == false);
    REQUIRE(q.pop() == 1);
    REQUIRE(q.empty() == true);
    q.push(2);
    q.push(3);
    REQUIRE(q.empty() == false);
    REQUIRE(q.pop() == 2);
    REQUIRE(q.empty() == false);
    REQUIRE(q.pop() == 3);
    REQUIRE(q.empty() == true);
}


TEST_CASE("Entity - index", "[unit][ecs]") {
    REQUIRE(EntityIndex(42) == EntityIndex(42));
    REQUIRE(EntityIndex(42) != EntityIndex(24));
    REQUIRE(EntityIndex(42) == 42);

    EntityIndex i(0);
    REQUIRE(i.value == 0);
    ++i;
    REQUIRE(i.value == 1);
    i++;
    REQUIRE(i.value == 2);
}


TEST_CASE("Entity - generation", "[unit][ecs]") {
    REQUIRE(EntityGeneration(42) == EntityGeneration(42));
    REQUIRE(EntityGeneration(42) != EntityGeneration(24));

    EntityGeneration g(0);
    REQUIRE(g.value == 0);
    ++g;
    REQUIRE(g.value > 0);
    auto v1 = g.value;
    g++;
    REQUIRE(g.value == v1 * 2);
}

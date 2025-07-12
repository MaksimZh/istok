// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <ecs/entity.hpp>

#include <unordered_set>


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
    REQUIRE(g.value == 1);
    g++;
    REQUIRE(g.value == 2);
}


TEST_CASE("Entity - value", "[unit][ecs]") {
    REQUIRE(
        Entity(EntityIndex(42), EntityGeneration(17)) ==
        Entity(EntityIndex(42), EntityGeneration(17)));
    REQUIRE(
        Entity(EntityIndex(42), EntityGeneration(17)) !=
        Entity(EntityIndex(24), EntityGeneration(17)));
    REQUIRE(
        Entity(EntityIndex(42), EntityGeneration(17)) !=
        Entity(EntityIndex(42), EntityGeneration(71)));

    Entity e(EntityIndex(42), EntityGeneration(17));
    REQUIRE(e.index() == EntityIndex(42));
    REQUIRE(e.generation() == EntityGeneration(17));
}


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


TEST_CASE("Entity - index pool", "[unit][ecs]") {
    EntityIndexPool pool(2);
    // use set to check if all indices are different
    std::unordered_set<uint64_t> indices;
    
    // reach limit
    for (int i = 0; i < 2; i++) {
        REQUIRE(pool.isFull() == false);
        indices.insert(pool.getFreeIndex().value);
    }
    REQUIRE(pool.isFull() == true);
    REQUIRE(indices.size() == 2);
    
    // extend and reach new limit
    pool.extendBy(3);
    for (int i = 0; i < 3; i++) {
        REQUIRE(pool.isFull() == false);
        indices.insert(pool.getFreeIndex().value);
    }
    REQUIRE(pool.isFull() == true);
    REQUIRE(indices.size() == 5);

    // check indices limits
    REQUIRE(*std::max_element(indices.begin(), indices.end()) < 5);
    
    // free some indices
    auto it = indices.begin();
    auto v1 = *(it++);
    auto v2 = *(it++);
    indices.erase(v1);
    indices.erase(v2);
    pool.freeIndex(EntityIndex(v1));
    pool.freeIndex(EntityIndex(v2));
    
    // reach limit once more
    for (int i = 0; i < 2; i++) {
        REQUIRE(pool.isFull() == false);
        indices.insert(pool.getFreeIndex().value);
    }
    REQUIRE(pool.isFull() == true);
    REQUIRE(indices.size() == 5);

    // check indices limits
    REQUIRE(*std::max_element(indices.begin(), indices.end()) < 5);
}


TEST_CASE("Entity - generation array", "[unit][ecs]") {
    //GenerationArray generations(2);
}

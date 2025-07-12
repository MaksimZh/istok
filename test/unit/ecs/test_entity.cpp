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
    GenerationArray generations(2);
    REQUIRE(generations[0] == EntityGeneration(0));
    REQUIRE(generations[1] == EntityGeneration(0));
    generations[1]++;
    REQUIRE(generations[1] == EntityGeneration(1));
    generations.extendBy(3);
    REQUIRE(generations[0] == EntityGeneration(0));
    REQUIRE(generations[1] == EntityGeneration(1));
    REQUIRE(generations[2] == EntityGeneration(0));
    REQUIRE(generations[3] == EntityGeneration(0));
    REQUIRE(generations[4] == EntityGeneration(0));
}


TEST_CASE("Entity - storage", "[unit][ecs]") {
    EntityStorage storage(2);
    // use set to check if all entities are different
    std::unordered_set<Entity, Entity::Hash> entities;
    
    REQUIRE(storage.size() == 2);
    REQUIRE(storage.isFull() == false);
    Entity e0 = storage.create();
    REQUIRE(storage.isFull() == false);
    REQUIRE(storage.isValid(e0) == true);
    Entity e1 = storage.create();
    REQUIRE(storage.isFull() == true);
    REQUIRE(storage.isValid(e1) == true);
    
    // check if entities are different
    entities.insert(e0);
    entities.insert(e1);
    REQUIRE(entities.size() == 2);

    // extend and reach new limit
    storage.extendBy(3);
    REQUIRE(storage.size() == 5);
    for (int i = 0; i < 3; i++) {
        REQUIRE(storage.isFull() == false);
        entities.insert(storage.create());
    }
    REQUIRE(storage.isFull() == true);
    REQUIRE(entities.size() == 5);

    // destroy some entities
    storage.destroy(e0);
    REQUIRE(storage.isFull() == false);
    REQUIRE(storage.isValid(e0) == false);
    storage.destroy(e1);
    REQUIRE(storage.isFull() == false);
    REQUIRE(storage.isValid(e1) == false);
    
    // create new entities
    Entity e2 = storage.create();
    Entity e3 = storage.create();
    REQUIRE(storage.isFull() == true);
    REQUIRE(storage.isValid(e0) == false);
    REQUIRE(storage.isValid(e1) == false);
    REQUIRE(storage.isValid(e2) == true);
    REQUIRE(storage.isValid(e3) == true);

    // check if all entities (valid and not) are different
    entities.insert(e2);
    entities.insert(e3);
    REQUIRE(entities.size() == 7);
}


TEST_CASE("Entity - manager", "[unit][ecs]") {
    EntityManager manager(2);
    // use set to check if all entities are different
    std::unordered_set<Entity, Entity::Hash> entities;

    // create much more entities than initial capacity
    for (int i = 0; i < 20; i++) {
        entities.insert(manager.create());
    }
    REQUIRE(entities.size() == 20);

    // check if all entities are valid
    for (const Entity& e : entities) {
        REQUIRE(manager.isValid(e) == true);
    }

    // remove some entities and ensure they became invalid
    auto it = entities.begin();
    Entity e0 = *it++;
    Entity e1 = *it++;
    manager.destroy(e0);
    manager.destroy(e1);
    REQUIRE(manager.isValid(e0) == false);
    REQUIRE(manager.isValid(e1) == false);

    // add more entities
    for (int i = 0; i < 20; i++) {
        entities.insert(manager.create());
    }
    REQUIRE(entities.size() == 20 + 20);

    // all removed ones are still invalid
    REQUIRE(manager.isValid(e0) == false);
    REQUIRE(manager.isValid(e1) == false);
}


TEST_CASE("Entity - manager deep remove", "[unit][ecs]") {
    EntityManager manager(2);
    // use set to check if all entities are different
    std::unordered_set<Entity, Entity::Hash> entities;

    for (int i = 0; i < 10; i++) {
        // all old entities (if any) must be invalid
        for (const Entity& e : entities) {
            REQUIRE(manager.isValid(e) == false);
        }
        Entity e0 = manager.create();
        Entity e1 = manager.create();
        entities.insert(e0);
        entities.insert(e1);
        REQUIRE(manager.isValid(e0));
        REQUIRE(manager.isValid(e1));
        manager.destroy(e0);
        manager.destroy(e1);
    }

    // all entities are different
    REQUIRE(entities.size() == 10 * 2);
}

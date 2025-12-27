// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include "helper.hpp"

#include <catch2/catch_test_macros.hpp>
#include <ecs/entity.hpp>

using namespace Istok::ECS;


TEST_CASE("Entity - value", "[unit][ecs]") {
    REQUIRE(Entity(42, 17) == Entity(42, 17));
    REQUIRE(Entity(42, 17) != Entity(24, 17));
    REQUIRE(Entity(42, 17) != Entity(42, 71));
    Entity e(42, 17);
    REQUIRE(e.index() == 42);
    REQUIRE(e.generation() == 17);
}


TEST_CASE("EntityStorage - create", "[unit][ecs]") {
    EntityStorage storage(2);
    REQUIRE(storage.capacity() == 2);
}


TEST_CASE("EntityStorage - createEntity", "[unit][ecs]") {
    EntityStorage storage(2);
    Entity a = storage.createEntity();
}


TEST_CASE("EntityStorage - destroyEntity", "[unit][ecs]") {
    EntityStorage storage(2);
    Entity a = storage.createEntity();
    storage.destroyEntity(a);
}


TEST_CASE("EntityStorage - extend", "[unit][ecs]") {
    EntityStorage storage(2);
    REQUIRE(storage.capacity() == 2);
    storage.extend(3);
    REQUIRE(storage.capacity() == 5);
    storage.extend(0);
    REQUIRE(storage.capacity() == 5);
}


TEST_CASE("EntityStorage - isFull", "[unit][ecs]") {
    EntityStorage storage(2);
    REQUIRE(storage.isFull() == false);  // TODO: full -> isFull
    storage.createEntity();
    REQUIRE(storage.isFull() == false);
    storage.createEntity();
    REQUIRE(storage.isFull() == true);
    storage.extend(3);
    storage.createEntity();
    REQUIRE(storage.isFull() == false);
    storage.createEntity();
    REQUIRE(storage.isFull() == false);
    storage.createEntity();
    REQUIRE(storage.isFull() == true);
    storage.extend(0);
    REQUIRE(storage.isFull() == true);
}


TEST_CASE("EntityStorage - isValidEntity", "[unit][ecs]") {
    EntityStorage storage(2);
    REQUIRE(storage.isValidEntity(Entity(0, 0)) == false);
    REQUIRE(storage.isValidEntity(Entity(0, 1)) == false);
    REQUIRE(storage.isValidEntity(Entity(0, 2)) == false);
    REQUIRE(storage.isValidEntity(Entity(1, 0)) == false);
    REQUIRE(storage.isValidEntity(Entity(1, 1)) == false);
    REQUIRE(storage.isValidEntity(Entity(1, 2)) == false);
    REQUIRE(storage.isValidEntity(Entity(2, 0)) == false);
    REQUIRE(storage.isValidEntity(Entity(2, 1)) == false);
    REQUIRE(storage.isValidEntity(Entity(2, 2)) == false);
    Entity a = storage.createEntity();
    Entity b = storage.createEntity();
    REQUIRE(storage.isValidEntity(a) == true);
    REQUIRE(storage.isValidEntity(b) == true);
    storage.destroyEntity(b);
    REQUIRE(storage.isValidEntity(a) == true);
    REQUIRE(storage.isValidEntity(b) == false);
    Entity c = storage.createEntity();
    REQUIRE(storage.isValidEntity(a) == true);
    REQUIRE(storage.isValidEntity(b) == false);
    REQUIRE(storage.isValidEntity(c) == true);
}


TEST_CASE("Entity - storage", "[unit][ecs]") {
    EntityStorage storage(3);
    REQUIRE(storage.capacity() == 3);
    REQUIRE(storage.isFull() == false);

    SECTION("create") {
        Entity e = storage.createEntity();
        REQUIRE(storage.isFull() == false);
        REQUIRE(storage.isValidEntity(e) == true);
    }

    SECTION("make full") {
        EntityUSet entities;
        entities.insert(storage.createEntity());
        entities.insert(storage.createEntity());
        entities.insert(storage.createEntity());
        REQUIRE(entities.size() == 3);
        REQUIRE(storage.isFull() == true);
        for (const Entity& e: entities) {
            REQUIRE(storage.isValidEntity(e) == true);
        }

        SECTION("extend") {
            storage.extend(2);
            REQUIRE(storage.capacity() == 5);
            REQUIRE(storage.isFull() == false);
            entities.insert(storage.createEntity());
            entities.insert(storage.createEntity());
            REQUIRE(storage.isFull() == true);
            REQUIRE(entities.size() == 5);
            for (const Entity& e: entities) {
                REQUIRE(storage.isValidEntity(e) == true);
            }
        }

        SECTION("destroy") {
            auto it = entities.begin();
            Entity e1 = *(it++);
            Entity e2 = *(it++);
            storage.destroyEntity(e1);
            storage.destroyEntity(e2);
            REQUIRE(storage.isFull() == false);
            REQUIRE(storage.isValidEntity(e1) == false);
            REQUIRE(storage.isValidEntity(e2) == false);

            SECTION("insert new") {
                entities.insert(storage.createEntity());
                entities.insert(storage.createEntity());
                REQUIRE(storage.isFull() == true);
                // all entities (old and new) are different
                REQUIRE(entities.size() == 5);
                entities.erase(e1);
                entities.erase(e2);
                for (const Entity& e: entities) {
                    REQUIRE(storage.isValidEntity(e) == true);
                }
            }
        }
    }

    SECTION("extend") {
        storage.extend(2);
        REQUIRE(storage.capacity() == 5);
        EntityUSet entities;
        entities.insert(storage.createEntity());
        entities.insert(storage.createEntity());
        entities.insert(storage.createEntity());
        REQUIRE(entities.size() == 3);
        REQUIRE(storage.isFull() == false);
        entities.insert(storage.createEntity());
        entities.insert(storage.createEntity());
        REQUIRE(storage.isFull() == true);
        REQUIRE(entities.size() == 5);
    }
}


TEST_CASE("Entity - manager", "[unit][ecs]") {
    EntityManager manager(2);
    
    SECTION("single entity") {
        Entity e = manager.create();
        REQUIRE(manager.isValid(e) == true);

        SECTION("destroy") {
            manager.destroy(e);
            REQUIRE(manager.isValid(e) == false);
        }
    }

    SECTION("many entities") {
        EntityUSet entities;
        // create much more entities than initial capacity
        for (int i = 0; i < 20; i++) {
            entities.insert(manager.create());
        }
        REQUIRE(entities.size() == 20);

        for (const Entity& e : entities) {
            REQUIRE(manager.isValid(e) == true);
        }

        SECTION("destroy") {
            auto it = entities.begin();
            Entity e1 = *(it++);
            Entity e2 = *(it++);
            manager.destroy(e1);
            manager.destroy(e2);
            REQUIRE(manager.isValid(e1) == false);
            REQUIRE(manager.isValid(e2) == false);

            SECTION("add more") {
                for (int i = 0; i < 20; i++) {
                    entities.insert(manager.create());
                }
                // all entities (old and new) are different
                REQUIRE(entities.size() == 20 + 20);

                entities.erase(e1);
                entities.erase(e2);
                REQUIRE(manager.isValid(e1) == false);
                REQUIRE(manager.isValid(e2) == false);
                for (const Entity& e : entities) {
                    REQUIRE(manager.isValid(e) == true);
                }
            }
        }
    }

    SECTION("mass destruction") {
        EntityUSet entities;
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
}

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
    EntityStorage storage(2);  // TODO: initialSize -> initialCapacity
    REQUIRE(storage.size() == 2);  // TODO: size -> capacity
}


TEST_CASE("EntityStorage - createEntity", "[unit][ecs]") {
    EntityStorage storage(2);
    Entity a = storage.create();  // TODO: create -> createEntity
}


TEST_CASE("EntityStorage - destroyEntity", "[unit][ecs]") {
    EntityStorage storage(2);
    Entity a = storage.create();
    storage.destroy(a);  // TODO: destroy -> destroyEntity
}


TEST_CASE("EntityStorage - extend", "[unit][ecs]") {
    EntityStorage storage(2);
    REQUIRE(storage.size() == 2);
    storage.extend(3);
    REQUIRE(storage.size() == 5);
    storage.extend(0);
    REQUIRE(storage.size() == 5);
}


TEST_CASE("EntityStorage - isFull", "[unit][ecs]") {
    EntityStorage storage(2);
    REQUIRE(storage.full() == false);  // TODO: full -> isFull
    storage.create();
    REQUIRE(storage.full() == false);
    storage.create();
    REQUIRE(storage.full() == true);
    storage.extend(3);
    storage.create();
    REQUIRE(storage.full() == false);
    storage.create();
    REQUIRE(storage.full() == false);
    storage.create();
    REQUIRE(storage.full() == true);
    storage.extend(0);
    REQUIRE(storage.full() == true);
}


TEST_CASE("EntityStorage - isValidEntity", "[unit][ecs]") {
    EntityStorage storage(2);
    REQUIRE(storage.isValid(Entity(0, 0)) == false);
    REQUIRE(storage.isValid(Entity(0, 1)) == false);
    REQUIRE(storage.isValid(Entity(0, 2)) == false);
    REQUIRE(storage.isValid(Entity(1, 0)) == false);
    REQUIRE(storage.isValid(Entity(1, 1)) == false);
    REQUIRE(storage.isValid(Entity(1, 2)) == false);
    // TODO: REQUIRE(storage.isValid(Entity(2, 0)) == false);
    // TODO: REQUIRE(storage.isValid(Entity(2, 1)) == false);
    // TODO: REQUIRE(storage.isValid(Entity(2, 2)) == false);
    Entity a = storage.create();
    Entity b = storage.create();
    REQUIRE(storage.isValid(a) == true);
    REQUIRE(storage.isValid(b) == true);
    storage.destroy(b);
    REQUIRE(storage.isValid(a) == true);
    REQUIRE(storage.isValid(b) == false);
    Entity c = storage.create();
    REQUIRE(storage.isValid(a) == true);
    REQUIRE(storage.isValid(b) == false);
    REQUIRE(storage.isValid(c) == true);
}


TEST_CASE("Entity - storage", "[unit][ecs]") {
    EntityStorage storage(3);
    REQUIRE(storage.size() == 3);
    REQUIRE(storage.full() == false);

    SECTION("create") {
        Entity e = storage.create();
        REQUIRE(storage.full() == false);
        REQUIRE(storage.isValid(e) == true);
    }

    SECTION("make full") {
        EntityUSet entities;
        entities.insert(storage.create());
        entities.insert(storage.create());
        entities.insert(storage.create());
        REQUIRE(entities.size() == 3);
        REQUIRE(storage.full() == true);
        for (const Entity& e: entities) {
            REQUIRE(storage.isValid(e) == true);
        }

        SECTION("extend") {
            storage.extend(2);
            REQUIRE(storage.size() == 5);
            REQUIRE(storage.full() == false);
            entities.insert(storage.create());
            entities.insert(storage.create());
            REQUIRE(storage.full() == true);
            REQUIRE(entities.size() == 5);
            for (const Entity& e: entities) {
                REQUIRE(storage.isValid(e) == true);
            }
        }

        SECTION("destroy") {
            auto it = entities.begin();
            Entity e1 = *(it++);
            Entity e2 = *(it++);
            storage.destroy(e1);
            storage.destroy(e2);
            REQUIRE(storage.full() == false);
            REQUIRE(storage.isValid(e1) == false);
            REQUIRE(storage.isValid(e2) == false);

            SECTION("insert new") {
                entities.insert(storage.create());
                entities.insert(storage.create());
                REQUIRE(storage.full() == true);
                // all entities (old and new) are different
                REQUIRE(entities.size() == 5);
                entities.erase(e1);
                entities.erase(e2);
                for (const Entity& e: entities) {
                    REQUIRE(storage.isValid(e) == true);
                }
            }
        }
    }

    SECTION("extend") {
        storage.extend(2);
        REQUIRE(storage.size() == 5);
        EntityUSet entities;
        entities.insert(storage.create());
        entities.insert(storage.create());
        entities.insert(storage.create());
        REQUIRE(entities.size() == 3);
        REQUIRE(storage.full() == false);
        entities.insert(storage.create());
        entities.insert(storage.create());
        REQUIRE(storage.full() == true);
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

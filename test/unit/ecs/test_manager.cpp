// test_manager.cpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include "helper.hpp"

#include <ecs/manager.hpp>

using namespace Istok::ECS;

#include <ranges>
#include <set>

namespace {
    struct A {
        int value;
        bool operator==(const A&) const = default;
    };

    struct B {
        int value;
        bool operator==(const B&) const = default;
    };

    struct C {
        int value;
        bool operator==(const C&) const = default;
    };
}


TEST_CASE("ECS - manager", "[unit][ecs]") {
    EntityComponentManager manager(3);

    SECTION("single entity") {
        Entity e0 = manager.createEntity();
        REQUIRE(manager.isValidEntity(e0) == true);
        REQUIRE(manager.hasComponent<A>(e0) == false);
        REQUIRE(manager.hasComponent<B>(e0) == false);
        REQUIRE(manager.hasComponent<C>(e0) == false);
        manager.addComponent(e0, A{0});
        REQUIRE(manager.hasComponent<A>(e0) == true);
        REQUIRE(manager.hasComponent<B>(e0) == false);
        REQUIRE(manager.hasComponent<C>(e0) == false);
        manager.addComponent(e0, B{0});
        REQUIRE(manager.hasComponent<A>(e0) == true);
        REQUIRE(manager.hasComponent<B>(e0) == true);
        REQUIRE(manager.hasComponent<C>(e0) == false);

        SECTION("remove components") {
            manager.removeComponent<A>(e0);
            REQUIRE(manager.hasComponent<A>(e0) == false);
            REQUIRE(manager.hasComponent<B>(e0) == true);
            REQUIRE(manager.hasComponent<C>(e0) == false);
            manager.removeComponent<B>(e0);
            REQUIRE(manager.hasComponent<A>(e0) == false);
            REQUIRE(manager.hasComponent<B>(e0) == false);
            REQUIRE(manager.hasComponent<C>(e0) == false);
        }

        SECTION("destroy entity") {
            manager.destroyEntity(e0);
            REQUIRE(manager.isValidEntity(e0) == false);
        }
    }

    SECTION("full storage") {
        Entity e0 = manager.createEntity();
        Entity e1 = manager.createEntity();
        Entity e2 = manager.createEntity();
        REQUIRE(manager.isValidEntity(e0) == true);
        REQUIRE(manager.isValidEntity(e1) == true);
        REQUIRE(manager.isValidEntity(e2) == true);
        REQUIRE(manager.hasComponent<A>(e0) == false);
        REQUIRE(manager.hasComponent<B>(e0) == false);
        REQUIRE(manager.hasComponent<C>(e0) == false);
        REQUIRE(manager.hasComponent<A>(e1) == false);
        REQUIRE(manager.hasComponent<B>(e1) == false);
        REQUIRE(manager.hasComponent<C>(e1) == false);
        REQUIRE(manager.hasComponent<A>(e2) == false);
        REQUIRE(manager.hasComponent<B>(e2) == false);
        REQUIRE(manager.hasComponent<C>(e2) == false);
        manager.addComponent(e0, A{0});
        manager.addComponent(e0, B{0});
        manager.addComponent(e1, B{1});
        manager.addComponent(e1, C{1});
        manager.addComponent(e2, A{2});
        manager.addComponent(e2, C{2});
        REQUIRE(manager.hasComponent<A>(e0) == true);
        REQUIRE(manager.hasComponent<B>(e0) == true);
        REQUIRE(manager.hasComponent<C>(e0) == false);
        REQUIRE(manager.hasComponent<A>(e1) == false);
        REQUIRE(manager.hasComponent<B>(e1) == true);
        REQUIRE(manager.hasComponent<C>(e1) == true);
        REQUIRE(manager.hasComponent<A>(e2) == true);
        REQUIRE(manager.hasComponent<B>(e2) == false);
        REQUIRE(manager.hasComponent<C>(e2) == true);

        SECTION("remove components") {
            manager.removeComponent<A>(e0);
            manager.removeComponent<B>(e1);
            manager.removeComponent<C>(e2);
            REQUIRE(manager.hasComponent<A>(e0) == false);
            REQUIRE(manager.hasComponent<B>(e0) == true);
            REQUIRE(manager.hasComponent<C>(e0) == false);
            REQUIRE(manager.hasComponent<A>(e1) == false);
            REQUIRE(manager.hasComponent<B>(e1) == false);
            REQUIRE(manager.hasComponent<C>(e1) == true);
            REQUIRE(manager.hasComponent<A>(e2) == true);
            REQUIRE(manager.hasComponent<B>(e2) == false);
            REQUIRE(manager.hasComponent<C>(e2) == false);
        }

        SECTION("destroy entity") {
            manager.destroyEntity(e0);
            manager.destroyEntity(e1);
            REQUIRE(manager.isValidEntity(e0) == false);
            REQUIRE(manager.isValidEntity(e1) == false);
            REQUIRE(manager.isValidEntity(e2) == true);
        }
    }

    SECTION("many entities") {
        EntityUSet entities;
        // create much more entities than initial capacity
        for (int i = 0; i < 20; i++) {
            entities.insert(manager.createEntity());
        }
        REQUIRE(entities.size() == 20);
        for (auto& e : entities) { REQUIRE(manager.isValidEntity(e) == true); }

        SECTION("destroy") {
            EntityUSet old;
            for (int i = 0; i < 20; i++) {
                Entity e = *entities.begin();
                manager.destroyEntity(e);
                entities.erase(e);
                old.insert(e);
            }

            for (auto e : old) { REQUIRE(manager.isValidEntity(e) == false); }
            for (auto e : entities) { REQUIRE(manager.isValidEntity(e) == true); }

            for (int i = 0; i < 20; i++) {
                entities.insert(manager.createEntity());
            }

            for (auto e : old) { REQUIRE(manager.isValidEntity(e) == false); }
            for (auto e : entities) { REQUIRE(manager.isValidEntity(e) == true); }
        }
    }
}

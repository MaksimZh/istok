// test_manager.cpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include "helper.hpp"

#include <ecs/manager.hpp>

using namespace Istok::ECS;


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
        REQUIRE(manager.entityExists(e0) == true);
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
    }
}

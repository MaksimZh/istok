// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <ecs/ecs.hpp>

namespace {
    Entity fakeEntity(size_t index) {
        return Entity(EntityIndex(index), EntityGeneration(0));
    }

    struct A { int value; };
    struct B { int value; };
    struct C { int value; };
}


TEST_CASE("ECS - component manager", "[unit][ecs]") {
    Entity e0 = fakeEntity(0);
    Entity e1 = fakeEntity(1);
    Entity e2 = fakeEntity(2);
    ComponentManager manager;
    REQUIRE(manager.has<A>(e0) == false);
    REQUIRE(manager.has<B>(e0) == false);
    REQUIRE(manager.has<C>(e0) == false);
    manager.add(e0, A{0});
    REQUIRE(manager.has<A>(e0) == true);
    REQUIRE(manager.has<B>(e0) == false);
    REQUIRE(manager.has<C>(e0) == false);
}

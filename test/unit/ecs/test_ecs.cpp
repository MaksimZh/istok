// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <ecs/ecs.hpp>

namespace {
    Entity fakeEntity(size_t index) {
        return Entity(EntityIndex(index), EntityGeneration(0));
    }
}


TEST_CASE("ECS - component manager", "[unit][ecs]") {
    Entity e0 = fakeEntity(0);
    Entity e1 = fakeEntity(1);
    Entity e2 = fakeEntity(2);
    ComponentManager manager;
}

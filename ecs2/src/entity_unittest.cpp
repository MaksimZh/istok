// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <catch2/catch_test_macros.hpp>
#include <ecs.hpp>

using namespace Istok::ECS;


TEST_CASE("Entity - create", "[unit][ecs]") {
    Entity e(42, 17);
    REQUIRE(e.index() == 42);
    REQUIRE(e.generation() == 17);
}

TEST_CASE("Entity - compare", "[unit][ecs]") {
    REQUIRE(Entity(42, 17) == Entity(42, 17));
    REQUIRE(Entity(42, 17) != Entity(24, 17));
    REQUIRE(Entity(42, 17) != Entity(42, 71));
}

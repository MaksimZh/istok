// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <catch2/catch_test_macros.hpp>
#include <ecs.hpp>

using namespace Istok::ECS;

TEST_CASE("ECSManager - entities", "[unit][ecs]") {
    ECSManager ecs;
    Entity a = ecs.createEntity();
    Entity b = ecs.createEntity();
    Entity c = ecs.createEntity();

    SECTION("entity validity") {
        REQUIRE(ecs.isValidEntity(a));
        REQUIRE(ecs.isValidEntity(b));
        REQUIRE(ecs.isValidEntity(c));
    }

    SECTION("entity uniqueness") {
        REQUIRE(a != b);
        REQUIRE(b != c);
        REQUIRE(c != a);
    }

    SECTION("invalidate on deletion") {
        ecs.deleteEntity(b);
        REQUIRE(ecs.isValidEntity(a));
        REQUIRE(!ecs.isValidEntity(b));
        REQUIRE(ecs.isValidEntity(c));
    }
}

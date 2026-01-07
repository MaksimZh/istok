// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <catch2/catch_test_macros.hpp>
#include <internal/component.hpp>

using namespace Istok::ECS;

TEST_CASE("ComponentStorageOf - basics", "[unit][ecs]") {
    ComponentStorageOf<int> cs;
    REQUIRE(cs.size() == 0);
    REQUIRE(!cs.has(0));
    REQUIRE(!cs.get(0));

    SECTION("insert first") {
        cs.insert(0, 42);
        REQUIRE(cs.has(0));
        REQUIRE(!cs.has(1));
        REQUIRE(cs.get(0));
        REQUIRE(!cs.get(1));
        REQUIRE(*cs.get(0) == 42);
    }

    SECTION("insert middle") {
        cs.insert(2, 42);
        REQUIRE(!cs.has(0));
        REQUIRE(!cs.has(1));
        REQUIRE(cs.has(2));
        REQUIRE(!cs.has(3));
        REQUIRE(!cs.get(0));
        REQUIRE(!cs.get(1));
        REQUIRE(cs.get(2));
        REQUIRE(!cs.get(3));
        REQUIRE(*cs.get(2) == 42);
    }

    SECTION("remove last") {
        cs.insert(0, 40);
        cs.insert(1, 41);
        cs.insert(2, 42);
        cs.insert(3, 43);
        cs.remove(3);
        REQUIRE(cs.has(0));
        REQUIRE(cs.has(1));
        REQUIRE(cs.has(2));
        REQUIRE(!cs.has(3));
        REQUIRE(*cs.get(0) == 40);
        REQUIRE(*cs.get(1) == 41);
        REQUIRE(*cs.get(2) == 42);
    }

    SECTION("remove middle") {
        cs.insert(0, 40);
        cs.insert(1, 41);
        cs.insert(2, 42);
        cs.insert(3, 43);
        cs.remove(1);
        REQUIRE(cs.has(0));
        REQUIRE(!cs.has(1));
        REQUIRE(cs.has(2));
        REQUIRE(cs.has(3));
        REQUIRE(*cs.get(0) == 40);
        REQUIRE(*cs.get(2) == 42);
        REQUIRE(*cs.get(3) == 43);
    }
}

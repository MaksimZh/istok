// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <catch2/catch_test_macros.hpp>
#include <internal/component.hpp>

using namespace Istok::ECS;

TEST_CASE("ComponentStorageOf - basics", "[unit][ecs]") {
    ComponentStorageOf<int> cs;
    REQUIRE(cs.size() == 0);
    REQUIRE(!cs.has(0));

    SECTION("insert first") {
        cs.insert(0, 42);
        REQUIRE(cs.has(0));
        REQUIRE(!cs.has(1));
        REQUIRE(cs.get(0) == 42);
    }

    SECTION("insert middle") {
        cs.insert(2, 42);
        REQUIRE(!cs.has(0));
        REQUIRE(!cs.has(1));
        REQUIRE(cs.has(2));
        REQUIRE(!cs.has(3));
        REQUIRE(cs.get(2) == 42);
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
        REQUIRE(cs.get(0) == 40);
        REQUIRE(cs.get(1) == 41);
        REQUIRE(cs.get(2) == 42);
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
        REQUIRE(cs.get(0) == 40);
        REQUIRE(cs.get(2) == 42);
        REQUIRE(cs.get(3) == 43);
    }
}


TEST_CASE("ComponentStorageOf - multi actions", "[unit][ecs]") {
    ComponentStorageOf<int> cs;
    cs.insert(0, 100);
    cs.insert(3, 103);
    cs.insert(2, 102);
    cs.insert(9, 109);
    cs.insert(6, 106);
    cs.insert(7, 107);
    REQUIRE(cs.has(0));
    REQUIRE(!cs.has(1));
    REQUIRE(cs.has(2));
    REQUIRE(cs.has(3));
    REQUIRE(!cs.has(4));
    REQUIRE(!cs.has(5));
    REQUIRE(cs.has(6));
    REQUIRE(cs.has(7));
    REQUIRE(!cs.has(8));
    REQUIRE(cs.has(9));
    REQUIRE(cs.get(0) == 100);
    REQUIRE(cs.get(2) == 102);
    REQUIRE(cs.get(3) == 103);
    REQUIRE(cs.get(6) == 106);
    REQUIRE(cs.get(7) == 107);
    REQUIRE(cs.get(9) == 109);
    cs.remove(2);
    cs.remove(9);
    cs.remove(6);
    cs.insert(0, 200);
    cs.insert(5, 205);
    cs.insert(2, 202);
    cs.insert(8, 208);
    REQUIRE(cs.has(0));
    REQUIRE(!cs.has(1));
    REQUIRE(cs.has(2));
    REQUIRE(cs.has(3));
    REQUIRE(!cs.has(4));
    REQUIRE(cs.has(5));
    REQUIRE(!cs.has(6));
    REQUIRE(cs.has(7));
    REQUIRE(cs.has(8));
    REQUIRE(!cs.has(9));
    REQUIRE(cs.get(0) == 200);
    REQUIRE(cs.get(2) == 202);
    REQUIRE(cs.get(3) == 103);
    REQUIRE(cs.get(5) == 205);
    REQUIRE(cs.get(7) == 107);
    REQUIRE(cs.get(8) == 208);
}

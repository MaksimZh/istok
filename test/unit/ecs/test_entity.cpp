// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <ecs/entity.hpp>


TEST_CASE("Entity - limited value", "[unit][ecs]") {
    LimitedCounter c(0, 2);
    REQUIRE(c == 0);
    REQUIRE(!c.isFull());
    ++c;
    REQUIRE(c == 1);
    REQUIRE(!c.isFull());
    c++;
    REQUIRE(c == 2);
    REQUIRE(c.isFull());
    c.extendBy(3);
    for (int i = 2; i < 5; i++) {
        REQUIRE(c == i);
        REQUIRE(!c.isFull());
        c++;
    }
    REQUIRE(c == 5);
    REQUIRE(c.isFull());
}

// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/core/tree.hpp>


TEST_CASE("Tree - NodeContainer", "[unit][gui]") {
    NodeContainer<int> nc;
    int a, b;
    REQUIRE(nc.contains(a) == false);
    REQUIRE(nc.contains(b) == false);
    nc.push_back(a);
    REQUIRE(nc.contains(a) == true);
    REQUIRE(nc.contains(b) == false);
    nc.push_back(b);
    REQUIRE(nc.contains(a) == true);
    REQUIRE(nc.contains(b) == true);
    nc.erase(a);
    REQUIRE(nc.contains(a) == false);
    REQUIRE(nc.contains(b) == true);
    nc.erase(b);
    REQUIRE(nc.contains(a) == false);
    REQUIRE(nc.contains(b) == false);
}

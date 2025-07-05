// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/core/tree.hpp>

#include <ranges>


TEST_CASE("Tree - NodeFilter", "[unit][gui]") {
    int a, b;
    NodeFilter<int> nc;
    REQUIRE(nc.contains(a) == false);
    REQUIRE(nc.contains(b) == false);
    nc.insert(a);
    REQUIRE(nc.contains(a) == true);
    REQUIRE(nc.contains(b) == false);
    nc.insert(b);
    REQUIRE(nc.contains(a) == true);
    REQUIRE(nc.contains(b) == true);
    nc.erase(a);
    REQUIRE(nc.contains(a) == false);
    REQUIRE(nc.contains(b) == true);
    nc.erase(b);
    REQUIRE(nc.contains(a) == false);
    REQUIRE(nc.contains(b) == false);
}


TEST_CASE("Tree - NodeContainer", "[unit][gui]") {
    int a, b, c, d;
    
    NodeContainer<int> nc;
    REQUIRE(nc.contains(a) == false);
    REQUIRE(nc.contains(b) == false);
    REQUIRE(nc.contains(c) == false);
    REQUIRE(nc.contains(d) == false);
    nc.push_back(a);
    REQUIRE(nc.contains(a) == true);
    REQUIRE(nc.contains(b) == false);
    REQUIRE(nc.contains(c) == false);
    REQUIRE(nc.contains(d) == false);
    nc.push_back(b);
    nc.push_back(c);
    nc.push_back(d);
    REQUIRE(nc.contains(a) == true);
    REQUIRE(nc.contains(b) == true);
    REQUIRE(nc.contains(c) == true);
    REQUIRE(nc.contains(d) == true);

    NodeFilter<int> nf;
    static_assert(std::forward_iterator<NodeContainer<int>::Range::Iterator>);
    REQUIRE(std::ranges::empty(nc.filter(nf)));
    nf.insert(a);
    nf.insert(c);
    REQUIRE(std::ranges::equal(nc.filter(nf), std::vector<int>{a, c}));
}

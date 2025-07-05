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
    REQUIRE(std::ranges::empty(nc.filter(nf)));
    nf.insert(a);
    nf.insert(c);
    REQUIRE(std::ranges::equal(nc.filter(nf), std::vector<int>{a, c}));
}


namespace {
    class FakeNode: public Node<FakeNode> {};
}


TEST_CASE("Tree - Node", "[unit][gui]") {
    FakeNode a, b;
    REQUIRE(a.getParent() == nullptr);
    REQUIRE(std::ranges::empty(a.getChildren()));
    REQUIRE(std::ranges::empty(a.getVisibleChildren()));
    REQUIRE(b.getParent() == nullptr);
    REQUIRE(std::ranges::empty(b.getChildren()));
    REQUIRE(std::ranges::empty(b.getVisibleChildren()));
    a.addChild(b);
    REQUIRE(a.getParent() == nullptr);
    REQUIRE(std::ranges::equal(
        a.getChildren()
            | std::views::transform([](FakeNode& n) { return &n; }),
        std::vector<FakeNode*>{&b}));
    REQUIRE(std::ranges::equal(
        a.getVisibleChildren()
            | std::views::transform([](FakeNode& n) { return &n; }),
        std::vector<FakeNode*>{&b}));
    REQUIRE(b.getParent() == &a);
    REQUIRE(std::ranges::empty(b.getChildren()));
    REQUIRE(std::ranges::empty(b.getVisibleChildren()));
}

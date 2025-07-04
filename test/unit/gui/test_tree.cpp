// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/core/tree.hpp>

#include <ranges>

namespace {
    struct _Marker;

    class FakeNode: private Node<FakeNode, _Marker> {
    public:
        using Node::getParent;
        using Node::getNext;
        using Node::getPrev;
        using Node::getChildren;
        using Node::addChild;
    };

    static_assert(std::forward_iterator<NodeIterator<FakeNode, _Marker>>);
    static_assert(std::ranges::forward_range<NodeRange<FakeNode, _Marker>>);
}


TEST_CASE("Tree - initial", "[unit][gui]") {
    FakeNode a;
    REQUIRE(a.getParent() == nullptr);
    REQUIRE(a.getNext() == nullptr);
    REQUIRE(a.getPrev() == nullptr);
    REQUIRE(std::ranges::empty(a.getChildren()));
}


TEST_CASE("Tree - add child", "[unit][gui]") {
    FakeNode a, b, c;
    a.addChild(b);
}

// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/core/tree.hpp>

#include <ranges>

namespace {
    class _FakeMarker1 {
    private:
        _FakeMarker1() = default;
    public:
        static _FakeMarker1 instance;
    };
    _FakeMarker1 marker1 = _FakeMarker1::instance;

    class FakeNode: private Node<FakeNode, _FakeMarker1> {};

    static_assert(std::forward_iterator<NodeIterator<FakeNode, _FakeMarker1>>);
    static_assert(std::ranges::forward_range<NodeRange<FakeNode, _FakeMarker1>>);
}


TEST_CASE("Tree - initial", "[unit][gui]") {
    FakeNode a;
}


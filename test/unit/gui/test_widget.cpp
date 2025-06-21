// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui\widget.hpp>

#include <ranges>


namespace {
    class FakeHandler: public WidgetHandler {};
}


TEST_CASE("Widget - handler", "[unit][gui]") {
    Widget widget;
    REQUIRE(widget.getHandler() == nullptr);
    widget.createHandler<FakeHandler>();
    WidgetHandler* handler = widget.getHandler();
    REQUIRE(handler != nullptr);
    REQUIRE(&(handler->getWidget()) == &widget);
}


TEST_CASE("Widget - attach/detach", "[unit][gui]") {
    Widget base;
    Widget part1;
    Widget part2;
    REQUIRE(base.getParts().size() == 0);
    REQUIRE(part1.getBase() == nullptr);
    REQUIRE(part2.getBase() == nullptr);
    attach(base, part1);
    REQUIRE(std::ranges::equal(base.getParts(), std::vector{&part1}));
    REQUIRE(part1.getBase() == &base);
    attach(base, part2);
    REQUIRE(std::ranges::equal(base.getParts(), std::vector{&part1, &part2}));
    REQUIRE(part2.getBase() == &base);
    detach(part1);
    REQUIRE(std::ranges::equal(base.getParts(), std::vector{&part2}));
    REQUIRE(part1.getBase() == nullptr);
    detach(part2);
    REQUIRE(base.getParts().size() == 0);
    REQUIRE(part2.getBase() == nullptr);
}

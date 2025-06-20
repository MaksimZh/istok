// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui\widget.hpp>


TEST_CASE("Widget - base", "[unit][gui]") {
    Widget base;
    Widget part;
    REQUIRE(part.getBase() == nullptr);
    part.setBase(&base);
    REQUIRE(part.getBase() == &base);
}


TEST_CASE("Widget - parts", "[unit][gui]") {
    Widget base;
    Widget part1;
    Widget part2;
    REQUIRE(base.numParts() == 0);
    REQUIRE(base.getParts().size() == 0);
    base.addPart(&part1);
    base.addPart(&part2);
    REQUIRE(base.numParts() == 2);
    REQUIRE(base.getParts()[0] == &part1);
    REQUIRE(base.getParts()[1] == &part2);
}


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

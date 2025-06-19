// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui\widget.hpp>


TEST_CASE("Widget - base", "[unit][gui]") {
    Widget widget1;
    Widget widget2;
    REQUIRE(widget1.getBase() == nullptr);
    REQUIRE(widget2.getBase() == nullptr);
    widget2.setBase(&widget1);
    REQUIRE(widget2.getBase() == &widget1);
}

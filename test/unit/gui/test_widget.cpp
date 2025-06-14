// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui\widget.hpp>

#include <format>


TEST_CASE("ImageWidget", "[unit][gui]") {
    ImageWidget img("button");
    REQUIRE(img.getKey() == "button");
}


TEST_CASE("TextWidget", "[unit][gui]") {
    TextWidget text("caption");
    REQUIRE(text.getText() == "caption");
}

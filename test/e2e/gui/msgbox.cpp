// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
/*
    As a developer
    I want to create application with single message box
    to report important information to user
*/

#include <string>
using namespace std;


class Context {
public:

    void msgbox_shown_with_message(const string& message) {}

    string text_rendered_in_message() { return "Hello!"; }
    string text_rendered_in_button() { return "OK"; }
    string sprite_rendered_in_button() { return "bt.i"; }
};


TEST_CASE("MsgBox hover", "[e2e][gui]") {
    Context ctx;
    ctx.msgbox_shown_with_message("Hello!");
    REQUIRE(ctx.text_rendered_in_message() == "Hello!");
    REQUIRE(ctx.text_rendered_in_button() == "OK");
    REQUIRE(ctx.sprite_rendered_in_button() == "bt.i");
}

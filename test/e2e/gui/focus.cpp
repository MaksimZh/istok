// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
/*
    As a user
    I want all app. windows I can interact look active when the app. is active
    to see what parts of the app. are ready for my commands
*/

#include <string>
using namespace std;


class Context {
public:
    void app_initialized() {}
    void app_deactivated() {}
    void window_created(const string& id) {}
    void window_activated(const string& id) {}
    bool window_looks_active(const string& id) {}
};


TEST_CASE("Single window activity", "[gui]") {
    Context ctx;
    // Given
    ctx.app_initialized();
    // When
    ctx.window_created("main");
    // Then
    REQUIRE(ctx.window_looks_active("main") == true);
    // When
    ctx.app_deactivated();
    // Then
    REQUIRE(ctx.window_looks_active("main") == false);
    // When
    ctx.window_activated("main");
    // Then
    REQUIRE(ctx.window_looks_active("main") == true);
}


TEST_CASE("Two window activity", "[gui]") {
    Context ctx;
    // Given
    ctx.app_initialized();
    // When
    ctx.window_created("main");
    ctx.window_created("tool");
    // Then
    REQUIRE(ctx.window_looks_active("main") == true);
    REQUIRE(ctx.window_looks_active("tool") == true);
    // When
    ctx.window_activated("main");
    // Then
    REQUIRE(ctx.window_looks_active("main") == true);
    REQUIRE(ctx.window_looks_active("tool") == true);
    // When
    ctx.window_activated("tool");
    // Then
    REQUIRE(ctx.window_looks_active("main") == true);
    REQUIRE(ctx.window_looks_active("tool") == true);
    // When
    ctx.app_deactivated();
    // Then
    REQUIRE(ctx.window_looks_active("main") == false);
    REQUIRE(ctx.window_looks_active("tool") == false);
    // When
    ctx.window_activated("main");
    // Then
    REQUIRE(ctx.window_looks_active("main") == true);
    REQUIRE(ctx.window_looks_active("tool") == true);
    // When
    ctx.app_deactivated();
    ctx.window_activated("tool");
    // Then
    REQUIRE(ctx.window_looks_active("main") == true);
    REQUIRE(ctx.window_looks_active("tool") == true);
}

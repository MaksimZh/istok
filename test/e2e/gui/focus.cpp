// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
/*
    As a user
    I want all app. windows I can interact look active when the app. is active
    to see what parts of the app. are ready for my commands
*/

#include <string>
#include <cassert>
#include <memory>

#include "gui/window.hpp"

using namespace std;


class MockSysWindow final {
public:
    MockSysWindow(
        const string& title, Rect<int> location,
        WindowEventListener& listener,
        MockSysWindow* parent)
    : listener(listener), decorActive(false) {}
    
    void emulateSetAppActive(bool active) {
        listener.onSetAppActive(active);
    }
    
    void emulateTrySetDecorActive(bool active) {
        if (listener.onTrySetDecorActive(active)) {
            decorActive = active;
        }
    }

    void sendTrySetDecorActive(bool active) {
        emulateTrySetDecorActive(active);
    }
    
    bool isDecorActive() {
        return decorActive;
    }

private:
    WindowEventListener& listener;
    bool decorActive;
};


class Context final {
public:
    Context() {}

    void app_deactivated() {
        windows.at(activeId)->getSysWindow().emulateSetAppActive(false);
        activeId = "";
    }

    void window_created(const string& id) {
        assert(id != "");
        assert(!windows.contains(id));
        windows[id] = move(windowManager.createWindow(id, {0, 0, 100, 100}));
        windows[id]->show();
        window_activated(id);
    }

    void window_activated(const string& id) {
        if (activeId != "") {
            windows.at(activeId)->getSysWindow().emulateTrySetDecorActive(false);
        }
        if (activeId == "") {
            windows.at(id)->getSysWindow().emulateSetAppActive(true);
        }
        windows.at(id)->getSysWindow().emulateTrySetDecorActive(true);
        activeId = id;
    }

    bool window_looks_active(const string& id) {
        return windows.at(id)->getSysWindow().isDecorActive();
    }

private:
    WindowManager<MockSysWindow> windowManager;
    map<string, unique_ptr<Window<MockSysWindow>>> windows;
    string activeId;
};


TEST_CASE("Single window activity", "[e2e][gui]") {
    // Given
    Context ctx;
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


TEST_CASE("Two window activity", "[e2e][gui]") {
    // Given
    Context ctx;
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

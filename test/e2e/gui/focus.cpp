// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
/*
    As a user
    I want all app. windows I can interact look active when the app. is active
    to see what parts of the app. are ready for my commands
*/

#include <string>
using namespace std;

class MockSysWindow {
public:
    MockSysWindow(WindowEventListener& listener)
    : listener(listener), decorActive(false) {}
    
    void emulateAppInactivate() {
        listener.onAppInactivate();
    }
    
    void emulateTrySetDecorActive(bool active) {
        if (listener.onTryDecorActive(active)) {
            decorActive = active;
        }
    }
    
    bool isDecorActive() {
        return decorActive;
    }

private:
    WindowEventListener& listener;
    bool decorActive;
};

class MockSysWindowFactory {
    
};

class Context {
public:
    Context() : windowManager(sysWindowFactory) {}

    void app_deactivated() {
        windows.at(activeId).getSysWindow() emulateAppInactivate();
        activeId = "";
    }

    void window_created(const string& id) {
        assert id != "";
        assert !windows.contains(id);
        windows[id] = windowManager.createWindow(id, {0, 0, 100, 100});
        windows[id].show();
        activeId = id;
    }

    void window_activated(const string& id) {
        windows.at(activeId).getSysWindow().emulateTrySetDecorActive(false);
        windows.at(id).getSysWindow().emulateTrySetDecorActive(true);
    }

    bool window_looks_active(const string& id) {
        return windows.at(activeId).getSysWindow().isDecorActive();
    }

private:
    MockSysWindowFactory sysWindowFactory;
    WindowManager<MockSysWindowFactory> windowManager;
    map<string, WinWindow> windows;
};


TEST_CASE("Single window activity", "[gui]") {
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


TEST_CASE("Two window activity", "[gui]") {
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

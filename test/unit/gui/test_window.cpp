// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include "gui/window.hpp"


class MockSysWindow {
public:
    MockSysWindow(
        const string& title, Rect<int> location,
        WindowEventListener& listener,
        MockSysWindow* parent) {}

    void sendTrySetDecorActive(bool active) {}
};


class MockActivityManager:
    public WindowActivityManager<WinWindow<MockSysWindow>> {
public:
    MockActivityManager() {}
    
    bool getAppActive() override {
        return true;
    }

    void setAppActive(bool active) override {}

    void removeWindow(WinWindow<MockSysWindow>* window) override {}
};


TEST_CASE("WinWindow activity", "[unit][gui]") {
    MockActivityManager activityManager;
    WinWindow<MockSysWindow> window(activityManager);
}

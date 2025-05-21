// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include "gui/window.hpp"

#include <vector>
#include <string>
#include <format>

using namespace std;


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
    MockActivityManager(vector<string>& log) : log(log) {}
    
    bool getAppActive() override {
        return true;
    }

    void setAppActive(bool active) override {}

    void removeWindow(WinWindow<MockSysWindow>* window) override {
        log.push_back(logRemove(window));
    }
    
    static string logRemove(WinWindow<MockSysWindow>* window) {
        return format("WAM.remove {:p}", (void*)window);
    }

private:
    vector<string>& log;
};


TEST_CASE("WinWindow remove from manager", "[unit][gui]") {
    vector<string> log;
    MockActivityManager activityManager(log);
    WinWindow<MockSysWindow>* windowPtr;
    {
        WinWindow<MockSysWindow> window(activityManager);
        windowPtr = &window;
        REQUIRE(log.size() == 0);
    }
    REQUIRE(log == vector<string>{
        MockActivityManager::logRemove(windowPtr)
    });
}

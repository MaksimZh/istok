// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include "gui/window.hpp"

#include <vector>
#include <string>
#include <format>
#include <memory>

using namespace std;

vector<string> fakeLog;

class MockSysWindow {
public:
    MockSysWindow(
        const string& title, Rect<int> location,
        WindowEventListener& listener,
        MockSysWindow* parent) : log(fakeLog) {}

    MockSysWindow(
        vector<string>& log,
        const string& title, Rect<int> location,
        WindowEventListener& listener,
        MockSysWindow* parent) : log(log) {}

    ~MockSysWindow() {
        log.push_back(logDestroy(this));
    }

    void sendTrySetDecorActive(bool active) {}

    static string logDestroy(MockSysWindow* sysWindow) {
        return format("SW.destroy {:p}", (void*)sysWindow);
    }

private:
    vector<string>& log;
};


class MockActivityManager:
    public WindowActivityManager<WinWindow<MockSysWindow>> {
public:
    MockActivityManager(vector<string>& log) : log(log) {}
    
    bool getAppActive() override {
        return appActive;
    }

    void setAppActive(bool active) override {
        appActive = active;
    }

    void removeWindow(WinWindow<MockSysWindow>* window) override {
        log.push_back(logRemove(window));
    }

    static string logRemove(WinWindow<MockSysWindow>* window) {
        return format("WAM.remove {:p}", (void*)window);
    }

private:
    vector<string>& log;
    bool appActive;
};


TEST_CASE("WinWindow activity", "[unit][gui]") {
    vector<string> log;
    MockActivityManager manager(log);
    WinWindow<MockSysWindow> window(manager);
    window.onSetAppActive(true);
    REQUIRE(manager.getAppActive() == true);
    REQUIRE(window.onTrySetDecorActive(true) == true);
    REQUIRE(window.onTrySetDecorActive(false) == false);
    window.onSetAppActive(false);
    REQUIRE(manager.getAppActive() == false);
    REQUIRE(window.onTrySetDecorActive(false) == true);
}


TEST_CASE("WinWindow destruction", "[unit][gui]") {
    vector<string> log;
    MockActivityManager manager(log);
    WinWindow<MockSysWindow>* windowPtr;
    MockSysWindow* sysWindowPtr;
    {
        WinWindow<MockSysWindow> window(manager);
        windowPtr = &window;
        auto sysWindow = make_unique<MockSysWindow>(
            log,
            "main", Rect<int>{0, 0, 100, 100}, window, nullptr);
        sysWindowPtr = sysWindow.get();
        window.setSysWindow(move(sysWindow));
        REQUIRE(&window.getSysWindow() == sysWindowPtr);
    }
    REQUIRE(log == vector<string>{
        MockActivityManager::logRemove(windowPtr),
        MockSysWindow::logDestroy(sysWindowPtr),
    });
}

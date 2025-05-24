// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include "gui/window.hpp"

#include <vector>
#include <string>
#include <format>
#include <memory>

using namespace std;


class MockSysWindow {
public:
    MockSysWindow(const string& title, vector<string>& log)
        : title(title), log(log) {}

    ~MockSysWindow() {
        log.push_back(logDestroy(title));
    }

    void sendTrySetDecorActive(bool active) {}

    const string& getTitle() { return title; }

    static string logDestroy(const string& title) {
        return format("SysWindow(\"{}\").destroy()", title);
    }

private:
    string title;
    vector<string>& log;
};


class MockSysWindowFactory {
public:
    MockSysWindowFactory(vector<string>& log) : log(log) {}
    
    unique_ptr<MockSysWindow> createSysWindow(
            const string& title, Rect<int> location,
            WindowEventListener& listener) {
        return make_unique<MockSysWindow>(title, log);
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
        auto sw = window->getSysWindow();
        string title = sw ? sw->getTitle() : "<null>";
        log.push_back(logRemove(title));
    }

    static string logRemove(const string& title) {
        return format("Manager.remove(\"{}\")", title);
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
    MockSysWindowFactory factory(log);
    MockActivityManager manager(log);
    WinWindow<MockSysWindow>* windowPtr;
    {
        WinWindow<MockSysWindow> window(manager);
        window.setSysWindow(
            factory.createSysWindow(
                "main", Rect<int>{0, 0, 100, 100}, window));
        REQUIRE(window.getSysWindow()->getTitle() == "main");
    }
    REQUIRE(log == vector<string>{
        MockActivityManager::logRemove("main"),
        MockSysWindow::logDestroy("main"),
    });
}


TEST_CASE("WindowManager activity", "[unit][gui]") {
    vector<string> log;
    MockSysWindowFactory factory(log);
    WindowManager<MockSysWindow, MockSysWindowFactory> manager(factory);
    unique_ptr<Window<MockSysWindow>> w1 =
        manager.createWindow("first", Rect<int>{0, 0, 100, 100});
    unique_ptr<Window<MockSysWindow>> w2 =
        manager.createWindow("second", Rect<int>{0, 0, 100, 100});
}

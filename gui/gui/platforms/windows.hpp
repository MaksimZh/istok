#pragma once

#include <memory>

#include "../window.hpp"

using namespace std;


class WinSysWindow {
public:
    WinSysWindow(WindowEventListener& listener) {}
};


template <typename SysWindow>
class WinWindow : public Window<SysWindow>, public WindowEventListener {
public:
    WinWindow() {}

    void onAppInactivate() override {}

    bool onTryDecorActive(bool active) override {
        return true;
    }

    SysWindow& getSysWindow() override {
        return *sysWindow;
    }

    void show() override {}

    
    void setSysWindow(unique_ptr<SysWindow>&& sysWindow) {
        this->sysWindow = move(sysWindow);
    }

private:
    unique_ptr<SysWindow> sysWindow;
};


template <typename SysWindow>
class WindowManager {
public:
    WindowManager() {}
    
    unique_ptr<Window<SysWindow>> createWindow(
            const string& title, Rect<int> location) {
        auto window = make_unique<WinWindow<SysWindow>>();
        auto sysWindow = make_unique<SysWindow>(title, location, *window);
        window->setSysWindow(move(sysWindow));
        return move(window);
    }
};

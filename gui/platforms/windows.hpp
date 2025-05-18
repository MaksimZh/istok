#pragma once

#include "gui/window.hpp"

class WinSysWindow {
public:
    WinSysWindow(WindowEventListener& listener) {}
};


template <typename SysWindow>
class WinWindow : public Window, public WindowEventListener {
public:
    WinWindow() {}
    
    void setSysWindow()

private:
    unique_ptr<SysWindow> sysWindow;
}


template <typename SysWindowFactory>
class WindowManager {
public:
    WindowManager(SysWindowFactory& sysWindowFactory)
    : sysWindowFactory(sysWindowFactory) {}
    
    Window createWindow(const string& title, Rect<int> location) {
        return WinWindow;
    }

private:
    SysWindowFactory& sysWindowFactory;
};
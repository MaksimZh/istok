#pragma once

#include <string>
#include <concepts>

using namespace std;


template <typename T>
struct Rect {
    T left;
    T top; 
    T right;
    T bottom;
};


class WindowEventListener {
public:
    virtual void onSetAppActive(bool active) = 0;
    virtual bool onTrySetDecorActive(bool active) = 0;
};


template <typename T>
concept SysWindow = requires(T sw) {
    T(string{}, Rect<int>{}, declval<WindowEventListener&>(), &sw);
    { sw.sendTrySetDecorActive(bool{}) } -> same_as<void>;
};


template <typename WindowType>
class WindowActivityManager {
public:
    virtual bool isAppActive() = 0;
    virtual void remove(WindowType* window) = 0;
};


template <SysWindow SW>
class Window {
public:
    virtual SW& getSysWindow() = 0;
    virtual void show() = 0;
};


template <SysWindow SW>
class WinWindow : public Window<SW>, public WindowEventListener {
public:
    WinWindow(WindowActivityManager<WinWindow>& activityManager)
    : activityManager(activityManager) {}

    void onSetAppActive(bool active) override {}

    bool onTrySetDecorActive(bool active) override {
        return true;
    }

    SW& getSysWindow() override {
        return *sysWindow;
    }

    void show() override {}

    
    void setSysWindow(unique_ptr<SW>&& sysWindow) {
        this->sysWindow = move(sysWindow);
    }

private:
    WindowActivityManager<WinWindow>& activityManager;
    unique_ptr<SW> sysWindow;
};


template <SysWindow SW>
class WindowManager : public WindowActivityManager<WinWindow<SW>> {
public:
    WindowManager() {}
    
    unique_ptr<Window<SW>> createWindow(
            const string& title, Rect<int> location) {
        auto window = make_unique<WinWindow<SW>>(*this);
        window->setSysWindow(make_unique<SW>(title, location, *window, nullptr));
        return move(window);
    }

    bool isAppActive() override {
        return true;
    }
    
    void remove(WinWindow<SW>* window) {}
};

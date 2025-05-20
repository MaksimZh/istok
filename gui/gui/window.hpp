#pragma once

#include <string>
#include <concepts>
#include <mutex>
#include <set>

using namespace std;

#define DISABLE_COPY_MOVE(ClassName) \
    ClassName(const ClassName&) = delete; \
    ClassName& operator=(const ClassName&) = delete; \
    ClassName(const ClassName&&) = delete; \
    ClassName& operator=(const ClassName&&) = delete;

template <typename T>
struct Rect {
    T left;
    T top; 
    T right;
    T bottom;
};


class WindowEventListener {
public:
    WindowEventListener() = default;
    virtual ~WindowEventListener() = default;
    DISABLE_COPY_MOVE(WindowEventListener)
    
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
    WindowActivityManager() = default;
    virtual ~WindowActivityManager() = default;
    DISABLE_COPY_MOVE(WindowActivityManager)

    virtual bool getAppActive() = 0;
    virtual void setAppActive(bool active) = 0;
    virtual void removeWindow(WindowType* window) = 0;
};


template <SysWindow SW>
class Window {
public:
    Window() {}
    virtual ~Window() = default;
    DISABLE_COPY_MOVE(Window)

    virtual SW& getSysWindow() = 0;
    virtual void show() = 0;
};


template <SysWindow SW>
class WinWindow final : public Window<SW>, public WindowEventListener {
public:
    WinWindow(WindowActivityManager<WinWindow>& activityManager)
    : activityManager(activityManager) {}

    ~WinWindow() {
        activityManager.removeWindow(this);
    }

    DISABLE_COPY_MOVE(WinWindow)

    void onSetAppActive(bool active) override {
        activityManager.setAppActive(active);
    }

    bool onTrySetDecorActive(bool active) override {
        return activityManager.getAppActive() == active;
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
class WindowManager final : public WindowActivityManager<WinWindow<SW>> {
public:
    WindowManager() {}

    DISABLE_COPY_MOVE(WindowManager)
    
    unique_ptr<Window<SW>> createWindow(
            const string& title, Rect<int> location) {
        auto window = make_unique<WinWindow<SW>>(*this);
        window->setSysWindow(make_unique<SW>(title, location, *window, nullptr));
        {
            lock_guard lock(windowMutex);
            windows.insert(window.get());
        }
        return move(window);
    }

    bool getAppActive() override {
        return appActive;
    }

    void setAppActive(bool active) override {
        if (appActive == active)
            return;
        appActive = active;
        lock_guard lock(windowMutex);
        for (WinWindow<SW>* window: windows) {
            window->getSysWindow().sendTrySetDecorActive(active);
        }
    }

    void removeWindow(WinWindow<SW>* window) override {
        lock_guard lock(windowMutex);
        windows.erase(window);
    }

private:
    bool appActive;
    set<WinWindow<SW>*> windows;
    mutex windowMutex;
};

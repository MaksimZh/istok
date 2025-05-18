#pragma once

template <typename T>
struct Rect {
    T left;
    T top; 
    T right;
    T bottom;
};


class WindowEventListener {
public:
    virtual void onAppInactivate() = 0;
    virtual bool onTryDecorActive(bool active) = 0;
};


template <typename SysWindow>
class Window {
public:
    virtual SysWindow& getSysWindow() = 0;
    virtual void show() = 0;
};

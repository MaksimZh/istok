// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <logging.hpp>

#include <windows.h>

namespace Istok::WinAPI {

template <typename T>
struct Rect {
    T left;
    T top;
    T right;
    T bottom;
};


struct WindowMessage {
    HWND hWnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
};

LRESULT handleMessageByDefault(const WindowMessage& message) noexcept;


class WindowMessageHandler {
public:
    virtual ~WindowMessageHandler() = default;
    virtual LRESULT handleMessage(WindowMessage message) noexcept = 0;
};

class WndHandle {
public:
    WndHandle() = default;
    WndHandle(Rect<int> screenLocation);
    ~WndHandle();

    WndHandle(const WndHandle&) = delete;
    WndHandle& operator=(const WndHandle&) = delete;
    WndHandle(WndHandle&& source);
    WndHandle& operator=(WndHandle&& source);

    operator bool() const noexcept { return !!hWnd_; }
    HWND getHWnd() const { return hWnd_; }

    void setHandler(WindowMessageHandler* handler);

private:
    CLASS_WITH_LOGGER_PREFIX("WinAPI", "WinAPI: ");
    HWND hWnd_;
    void clean();
};

}  // namespace Istok::WinAPI

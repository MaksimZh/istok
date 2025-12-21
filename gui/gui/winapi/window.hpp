// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <logging.hpp>

#include <windows.h>

namespace Istok::GUI::WinAPI {

HINSTANCE getHInstance();

class WindowClass {
public:
    WindowClass() = default;
    WindowClass(WNDPROC lpfnWndProc, LPCWSTR className);
    ~WindowClass() noexcept;

    WindowClass(const WindowClass&) = delete;
    WindowClass& operator=(const WindowClass&) = delete;
    WindowClass(WindowClass&& other) = delete;
    WindowClass& operator=(WindowClass&& other) = delete;

    LPCWSTR get() const {
        return name;
    }

private:
    LPCWSTR name = nullptr;
};


struct WindowMessage {
    HWND hWnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
};


LRESULT handleByDefault(WindowMessage message);


class WindowMessageHandler {
public:
    virtual ~WindowMessageHandler() = default;
    virtual LRESULT handleWindowMessage(WindowMessage message) noexcept = 0;
};


class WndHandle {
public:
    WndHandle() = default;
    WndHandle(HWND hWnd) : hWnd_(hWnd) {}
    ~WndHandle();

    WndHandle(const WndHandle&) = delete;
    WndHandle& operator=(const WndHandle&) = delete;
    
    WndHandle(WndHandle&& source);
    WndHandle& operator=(WndHandle&& source);

    operator bool() const noexcept {
        return hWnd_;
    }

    HWND getHWnd() const {
        return hWnd_;
    }

    void setHandler(WindowMessageHandler* handler);

private:
    CLASS_WITH_LOGGER("Windows");
    
    HWND hWnd_ = nullptr;

    void drop();
    void clear();
};

} // namespace Istok::GUI::WinAPI

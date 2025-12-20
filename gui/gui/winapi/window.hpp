// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <tools/exchange.hpp>
#include <logging.hpp>

#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>

namespace Istok::GUI::WinAPI {


inline HINSTANCE getHInstance() {
    static HINSTANCE hInstance =
        reinterpret_cast<HINSTANCE>(GetModuleHandle(NULL));
    return hInstance;
}


class WindowClass {
public:
    WindowClass() = default;

    WindowClass(WNDPROC lpfnWndProc, LPCWSTR className)
    : name(className) {
        WNDCLASSEX wcex{};
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_OWNDC;
        wcex.lpfnWndProc = lpfnWndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = getHInstance();
        wcex.hIcon = nullptr;
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = nullptr;
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = className;
        wcex.hIconSm = nullptr;
        if (!RegisterClassEx(&wcex)) {
            throw std::runtime_error("Failed to register window class.");
        }
    }

    ~WindowClass() noexcept {
        UnregisterClass(name, getHInstance());
    }

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


inline LRESULT handleByDefault(WindowMessage message) {
    return DefWindowProc(
        message.hWnd,
        message.msg,
        message.wParam,
        message.lParam);
}


class WindowMessageHandler {
public:
    virtual ~WindowMessageHandler() = default;
    virtual LRESULT handleWindowMessage(WindowMessage message) noexcept = 0;
};


class WndHandle {
public:
    WndHandle() = default;
    
    WndHandle(HWND hWnd) : hWnd_(hWnd) {}

    ~WndHandle() {
        clear();
    }

    WndHandle(const WndHandle&) = delete;
    WndHandle& operator=(const WndHandle&) = delete;
    
    WndHandle(WndHandle&& source) : hWnd_(source.hWnd_) {
        source.drop();
    }
    
    WndHandle& operator=(WndHandle&& source) {
        if (this != &source) {
            clear();
            hWnd_ = source.hWnd_;
            source.drop();
        }
        return *this;
    }

    operator bool() const noexcept {
        return hWnd_;
    }

    HWND getHWnd() const {
        return hWnd_;
    }

    void setHandler(WindowMessageHandler* handler) {
        SetWindowLongPtr(
            hWnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(handler));
    }

private:
    CLASS_WITH_LOGGER("Windows");
    
    HWND hWnd_ = nullptr;

    void drop() {
        hWnd_ = nullptr;
    }

    void clear() {
        if (hWnd_) {
            LOG_DEBUG("Destroying window: {}", (void*)hWnd_);
            setHandler(nullptr);
            DestroyWindow(hWnd_);
            drop();
        }
    }
};

} // namespace Istok::GUI::WinAPI

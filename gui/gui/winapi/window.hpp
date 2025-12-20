// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <tools/exchange.hpp>

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


} // namespace Istok::GUI::WinAPI

// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "real_winapi.hpp"

#include <windows.h>

#include <istok/logging.hpp>

#include "base/message.hpp"
#include "base/windowproc.hpp"

namespace Istok::GUI::WinAPI {

namespace {

LRESULT defWindowProc(const WindowMessage& message) noexcept {
    return DefWindowProc(
        message.hWnd, message.msg, message.wParam, message.lParam);
}

struct HandlerStorage {
    inline static WindowMessageHandler defHandler{defWindowProc};

    static WindowMessageHandler& get(HWND hWnd) noexcept {
        LONG_PTR handler = GetWindowLongPtr(hWnd, GWLP_USERDATA);
        return handler
            ? *reinterpret_cast<WindowMessageHandler*>(handler)
            : defHandler;
    }
};

HINSTANCE getHInstance() {
    static HINSTANCE hInstance =
        reinterpret_cast<HINSTANCE>(GetModuleHandle(NULL));
    return hInstance;
}

class WindowClass {
public:
    WindowClass() = default;

    WindowClass(WNDPROC lpfnWndProc, LPCWSTR className) noexcept
    : name(className) {
        WITH_LOGGER_PREFIX("Istok.GUI.WinAPI", "WinAPI: ");
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
            LOG_ERROR("Failed to register window class.");
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

}  // namespace


HWND RealWinAPI::createWindow(const Rect<int>& location) noexcept {
    static WinAPI::WindowClass windowClass(
        windowProc<HandlerStorage>,
        L"Istok");
    return CreateWindowEx(
        NULL,
        windowClass.get(),
        L"Istok",
        WS_OVERLAPPEDWINDOW,
        location.left,
        location.top,
        location.right - location.left,
        location.bottom - location.top,
        NULL, NULL, WinAPI::getHInstance(), nullptr);
}

void RealWinAPI::destroyWindow(HWND hWnd) noexcept {
    DestroyWindow(hWnd);
}

LRESULT RealWinAPI::defWindowProc(const WindowMessage& message) noexcept {
    return DefWindowProc(
        message.hWnd, message.msg, message.wParam, message.lParam);
}

void RealWinAPI::setRawUserPointer(HWND hWnd, LONG_PTR ptr) noexcept {
    SetWindowLongPtr(hWnd, GWLP_USERDATA, ptr);
}

void RealWinAPI::getMessage(MSG& msg) noexcept {
    GetMessage(&msg, nullptr, 0, 0);
}

void RealWinAPI::dispatchMessage(const MSG& msg) noexcept {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
}

void RealWinAPI::showWindow(HWND hWnd) noexcept {
    ShowWindow(hWnd, SW_SHOW);
}

}  // namespace Istok::GUI::WinAPI

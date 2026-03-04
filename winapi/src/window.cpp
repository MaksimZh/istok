// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "istok/winapi/window.hpp"

#include <memory>
#include <set>
#include <string>
#include <windows.h>
#include <windowsx.h>

#include "istok/winapi/messages.hpp"


namespace Istok::WinAPI {

namespace {

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
        WITH_LOGGER_PREFIX("Windows", "WinAPI: ");
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

void logWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static std::set<UINT> mouseMessages = {
        WM_ENTERIDLE,
        WM_SETCURSOR,
        WM_MOUSEMOVE,
        WM_NCHITTEST,
        WM_NCMOUSEMOVE,
        WM_NCMOUSELEAVE,
    };
    if (mouseMessages.contains(msg)) {
        WITH_LOGGER_PREFIX("WinAPI.WndProc.MouseMove", "WndProc: ");
        LOG_TRACE("{}", formatMessage(hWnd, msg, wParam, lParam));
    } else {
        WITH_LOGGER_PREFIX("WinAPI.WndProc", "WndProc: ");
        LOG_TRACE("{}", formatMessage(hWnd, msg, wParam, lParam));
    }
}

LRESULT CALLBACK windowProc(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
) noexcept {
    logWindowProc(hWnd, msg, wParam, lParam);
    if (auto* handler = reinterpret_cast<WinAPI::WindowMessageHandler*>(
            GetWindowLongPtr(hWnd, GWLP_USERDATA))
    ) {
        return (*handler)(WinAPI::WindowMessage(hWnd, msg, wParam, lParam));
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

}  // namespace


WndHandle::WndHandle(Rect<int> screenLocation) {
    static WinAPI::WindowClass windowClass(windowProc, L"Istok");
    HWND hWnd = CreateWindowEx(
        NULL,
        windowClass.get(),
        L"Istok",
        WS_OVERLAPPEDWINDOW,
        screenLocation.left, screenLocation.top,
        screenLocation.right - screenLocation.left,
        screenLocation.bottom - screenLocation.top,
        NULL, NULL, WinAPI::getHInstance(), nullptr);
    if (!hWnd) {
        LOG_ERROR("Window creation failed.");
        return;
    }
    LOG_DEBUG("Created window: [{}]", toString(hWnd));
    hWnd_ = hWnd;
}

WndHandle::~WndHandle() {
    clean();
}

WndHandle::WndHandle(WndHandle&& source) {
    takeFrom(source);
}

WndHandle& WndHandle::operator=(WndHandle&& source) {
    if (&source != this) {
        clean();
        takeFrom(source);
    }
    return *this;
}

void WndHandle::setMessageHandler(WindowMessageHandler&& handler) {
    if (!hWnd_) {
        return;
    }
    handler_ = std::make_unique<WindowMessageHandler>(std::move(handler));
    SetWindowLongPtr(
        hWnd_, GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(handler_.get()));
}

void WndHandle::resetMessageHandler() {
    if (!hWnd_ || !handler_) {
        return;
    }
    SetWindowLongPtr(hWnd_, GWLP_USERDATA, NULL);
    handler_.reset();
}

void WndHandle::takeFrom(WndHandle& source) {
    hWnd_ = source.hWnd_;
    handler_ = std::move(source.handler_);
    source.hWnd_ = nullptr;
}

void WndHandle::clean() {
    if (!hWnd_) {
        return;
    }
    LOG_DEBUG("Destroying window [{}]", toString(hWnd_));
    SetWindowLongPtr(hWnd_, GWLP_USERDATA, NULL);
    DestroyWindow(hWnd_);
}

}  // namespace Istok::WinAPI

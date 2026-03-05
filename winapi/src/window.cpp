// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "istok/winapi/window.hpp"

#include <memory>
#include <string>
#include <windows.h>

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

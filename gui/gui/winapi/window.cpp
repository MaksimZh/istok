// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "window.hpp"

namespace Istok::GUI::WinAPI {

HINSTANCE getHInstance() {
    static HINSTANCE hInstance =
        reinterpret_cast<HINSTANCE>(GetModuleHandle(NULL));
    return hInstance;
}


WindowClass::WindowClass(WNDPROC lpfnWndProc, LPCWSTR className)
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

WindowClass::~WindowClass() noexcept {
    UnregisterClass(name, getHInstance());
}


LRESULT handleByDefault(WindowMessage message) {
    return DefWindowProc(
        message.hWnd,
        message.msg,
        message.wParam,
        message.lParam);
}


WndHandle::~WndHandle() {
    clear();
}

WndHandle::WndHandle(WndHandle&& source) : hWnd_(source.hWnd_) {
    source.drop();
}
    
WndHandle& WndHandle::operator=(WndHandle&& source) {
    if (this != &source) {
        clear();
        hWnd_ = source.hWnd_;
        source.drop();
    }
    return *this;
}

void WndHandle::setHandler(WindowMessageHandler* handler) {
    SetWindowLongPtr(
        hWnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(handler));
}

void WndHandle::drop() {
    hWnd_ = nullptr;
}

void WndHandle::clear() {
    if (hWnd_) {
        LOG_DEBUG("Destroying window: {}", (void*)hWnd_);
        setHandler(nullptr);
        DestroyWindow(hWnd_);
        drop();
    }
}

} // namespace Istok::GUI::WinAPI

// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "window.hpp"

#include <memory>

#include <dwmapi.h>
#include <winuser.h>


namespace Istok::GUI::WinAPI {

namespace {

HINSTANCE getHInstance() {
    static HINSTANCE hInstance =
        reinterpret_cast<HINSTANCE>(GetModuleHandle(NULL));
    return hInstance;
}

class WindowClass {
public:
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

LRESULT CALLBACK windowProc(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
) noexcept {
    if (WinAPI::WindowMessageHandler* handler =
            reinterpret_cast<WinAPI::WindowMessageHandler*>(
                GetWindowLongPtr(hWnd, GWLP_USERDATA))
    ) {
        return handler->handleWindowMessage(
            WinAPI::WindowMessage(hWnd, msg, wParam, lParam));
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool enableTransparency(HWND hWnd) noexcept {
    DWM_BLURBEHIND bb = { 0 };
    HRGN hRgn = CreateRectRgn(0, 0, -1, -1);
    bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
    bb.hRgnBlur = hRgn;
    bb.fEnable = TRUE;
    return DwmEnableBlurBehindWindow(hWnd, &bb) == S_OK;
}

bool prepareForGL(HWND hWnd) noexcept {
    DCHandle dc(hWnd);
    if (!dc) {
        return false;
    }
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags =
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL |
        PFD_DOUBLEBUFFER |
        PFD_SUPPORT_COMPOSITION;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pfi = ChoosePixelFormat(dc.getDC(), &pfd);
    if (!pfi) {
        return false;
    }
    return SetPixelFormat(dc.getDC(), pfi, &pfd);
}

}  // namespace


LRESULT WindowMessage::handleByDefault() noexcept {
    return DefWindowProc(hWnd, msg, wParam, lParam);
}


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
        throw std::runtime_error("Cannot create window");
    }
    enableTransparency(hWnd);
    prepareForGL(hWnd);
    hWnd_ = std::make_unique<HWND>(hWnd);
}


WndHandle::~WndHandle() {
    if (hWnd_ && *hWnd_) {
        LOG_DEBUG("Destroying window: {}", (void*)getHWnd());
        setHandler(nullptr);
        DestroyWindow(getHWnd());
    }
}


void WndHandle::setHandler(WindowMessageHandler* handler) {
    SetWindowLongPtr(
        getHWnd(), GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(handler));
}


DCHandle::DCHandle(HWND hWnd)
: hDC_(std::make_unique<HDC>(GetDC(hWnd))) {}

DCHandle::~DCHandle() {
    if (hDC_ && *hDC_) {
        ReleaseDC(WindowFromDC(*hDC_), *hDC_);
    }
}


} // namespace Istok::GUI::WinAPI

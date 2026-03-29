// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "window.hpp"

#include <memory>

#include <windows.h>
#include <dwmapi.h>

#include "winapi/core2/message.hpp"


namespace Istok::GUI::WinAPI {

namespace {

LRESULT defWindowProc(const WindowMessage& message) noexcept {
    return DefWindowProc(
        message.hWnd, message.msg, message.wParam, message.lParam);
}

LRESULT CALLBACK windowProc(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
) noexcept {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI.WndProc", "WndProc: ");
    WindowMessage message{hWnd, msg, wParam, lParam};
    LONG_PTR handler = GetWindowLongPtr(hWnd, GWLP_USERDATA);
    LOG_TRACE("{} {}", handler ? "+" : "-", message);
    return handler
        ? (*reinterpret_cast<WindowMessageHandler*>(handler))(message)
        : defWindowProc(message);
}

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

bool enableTransparency(HWND hWnd) noexcept {
    WITH_LOGGER_PREFIX("Windows", "WinAPI: ");
    DWM_BLURBEHIND bb = { 0 };
    HRGN hRgn = CreateRectRgn(0, 0, -1, -1);
    bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
    bb.hRgnBlur = hRgn;
    bb.fEnable = TRUE;
    if (DwmEnableBlurBehindWindow(hWnd, &bb) != S_OK) {
        LOG_ERROR("DwmEnableBlurBehindWindow failed for window {}.", hWnd);
        return false;
    }
    return true;
}

bool setPixelFormatForGL(HWND hWnd) noexcept {
    WITH_LOGGER_PREFIX("Windows", "WinAPI: ");
    HDC hDC = GetDC(hWnd);
    if (!hDC) {
        LOG_ERROR("GetDC failed for window {}.", hWnd);
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

    int pfi = ChoosePixelFormat(hDC, &pfd);
    if (!pfi) {
        LOG_ERROR("ChoosePixelFormat failed for window {}.", hWnd);
        return false;
    }
    if (!SetPixelFormat(hDC, pfi, &pfd)) {
        LOG_ERROR("SetPixelFormat failed for window {}.", hWnd);
        return false;
    }
    return true;
}

}  // namespace


Window::Window(Rect<int> location, WindowMessageHandler&& handler) noexcept {
    static WinAPI::WindowClass windowClass(
        windowProc,
        L"Istok");
    HWND hWnd = CreateWindowEx(
        NULL,
        windowClass.get(),
        L"Istok",
        WS_OVERLAPPEDWINDOW,
        location.left,
        location.top,
        location.right - location.left,
        location.bottom - location.top,
        NULL, NULL, WinAPI::getHInstance(), nullptr);
    if (!hWnd) {
        LOG_ERROR("Failed to create window.");
        return;
    }
    enableTransparency(hWnd);
    setPixelFormatForGL(hWnd);
    LOG_DEBUG("Created window: {}", hWnd);
    hWnd_ = hWnd;
    handler_ = std::make_unique<WindowMessageHandler>(std::move(handler));
    SetWindowLongPtr(
        hWnd, GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(handler_.get()));
}

Window::~Window() {
    clear();
}

Window::Window(Window&& source) {
    takeFrom(source);
}

Window& Window::operator=(Window&& source) {
    if (&source != this) {
        clear();
        takeFrom(source);
    }
    return *this;
}

void Window::takeFrom(Window& source) {
    hWnd_ = source.hWnd_;
    handler_ = std::move(source.handler_);
    source.hWnd_ = nullptr;
}

void Window::clear() {
    if (!hWnd_) {
        return;
    }
    LOG_DEBUG("Destroying window {}", hWnd_);
    SetWindowLongPtr(hWnd_, GWLP_USERDATA, NULL);
    DestroyWindow(hWnd_);
    hWnd_ = nullptr;
}

}  // namespace Istok::GUI::WinAPI
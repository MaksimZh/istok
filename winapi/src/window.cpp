// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "logging.hpp"
#include <internal/window.hpp>
#include <type_traits>
#include <winuser.h>

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

template <typename T>
requires std::is_pointer_v<T>
std::string toString(T x) noexcept {
    return std::format("{:#x}", reinterpret_cast<uintptr_t>(x));
}

std::string fancyMessage(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
) noexcept {
    switch (msg) {
    case 0x0001: return std::format("WM_CREATE({})", toString(hWnd));
    case 0x0002: return std::format("WM_DESTROY({})", toString(hWnd));
    case 0x0003: return std::format("WM_MOVE({})", toString(hWnd));
    case 0x0005: return std::format("WM_SIZE({})", toString(hWnd));
    case 0x0006: return std::format("WM_ACTIVATE({})", toString(hWnd));
    case 0x0007: return std::format("WM_SETFOCUS({})", toString(hWnd));
    case 0x0010: return std::format("WM_CLOSE({})", toString(hWnd));
    case 0x0018: return std::format(
        "WM_SHOWWINDOW({}, wParam={:s}, lParam={:d})",
        toString(hWnd), static_cast<bool>(wParam), lParam);
    case 0x001c: return std::format("WM_ACTIVATEAPP({})", toString(hWnd));
    case 0x0024: return std::format("WM_GETMINMAXINFO({})", toString(hWnd));
    case 0x0046: return std::format("WM_WINDOWPOSCHANGING({})", toString(hWnd));
    case 0x007f: return std::format("WM_GETICON({})", toString(hWnd));
    case 0x0081: return std::format("WM_NCCREATE({})", toString(hWnd));
    case 0x0083: return std::format(
        "WM_NCCALCSIZE({}, wParam={:s})",
        toString(hWnd), static_cast<bool>(wParam));
    case 0x0085: return std::format("WM_NCPAINT({})", toString(hWnd));
    case 0x0086: return std::format("WM_NCACTIVATE({})", toString(hWnd));
    default: return std::format(
        "WindowMessage({}, {:#x}, {:#x}, {:#x})",
        toString(hWnd), msg, wParam, lParam);
    }
}

LRESULT CALLBACK windowProc(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
) noexcept {
    WITH_LOGGER_PREFIX("WinAPI", "WinAPI: ");
    LOG_TRACE("windowProc <- {}", fancyMessage(hWnd, msg, wParam, lParam));
    if (auto* handler = reinterpret_cast<WinAPI::WindowMessageHandler*>(
            GetWindowLongPtr(hWnd, GWLP_USERDATA))
    ) {
        return handler->handleMessage(
            WinAPI::WindowMessage(hWnd, msg, wParam, lParam));
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

}  // namespace


LRESULT handleMessageByDefault(const WindowMessage& message) noexcept {
    return DefWindowProcW(
        message.hWnd, message.msg, message.wParam, message.lParam);
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
        LOG_ERROR("Window creation failed.");
        return;
    }
    LOG_DEBUG("Created window: {}", toString(hWnd));
    hWnd_ = hWnd;
}

WndHandle::~WndHandle() {
    clean();
}

WndHandle::WndHandle(WndHandle&& source) {
    hWnd_ = source.hWnd_;
    source.hWnd_ = nullptr;
}

WndHandle& WndHandle::operator=(WndHandle&& source) {
    if (&source != this) {
        clean();
        hWnd_ = source.hWnd_;
        source.hWnd_ = nullptr;
    }
    return *this;
}

void WndHandle::setHandler(WindowMessageHandler* handler) {
    LOG_DEBUG("Set handler {} for window {}",
        toString(handler), toString(hWnd_));
    SetWindowLongPtr(
        hWnd_, GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(handler));
}

void WndHandle::clean() {
    if (hWnd_) {
        LOG_DEBUG("Destroying window: {}", toString(hWnd_));
        setHandler(nullptr);
        DestroyWindow(hWnd_);
    }
}

}  // namespace Istok::WinAPI

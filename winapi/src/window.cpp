// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <internal/window.hpp>

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

LRESULT CALLBACK windowProc(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
) noexcept {
    if (WinAPI::WindowMessageHandler* handler =
            reinterpret_cast<WinAPI::WindowMessageHandler*>(
                GetWindowLongPtr(hWnd, GWLP_USERDATA))
    ) {
        return handler->handleMessage(
            WinAPI::WindowMessage(hWnd, msg, wParam, lParam));
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
    LOG_TRACE("Created window: {}", static_cast<void*>(hWnd));
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
    SetWindowLongPtr(
        hWnd_, GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(handler));
}

void WndHandle::clean() {
    if (hWnd_) {
        LOG_TRACE("Destroying window: {}", static_cast<void*>(hWnd_));
        setHandler(nullptr);
        DestroyWindow(hWnd_);
    }
}


}  // namespace Istok::WinAPI

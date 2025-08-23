// window.hpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "platform.hpp"

#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>

#include <memory>
#include <unordered_map>


namespace Istok::GUI::WinAPI {

class WindowClass {
public:
    WindowClass() = default;

    WindowClass(
        UINT style,
        WNDPROC lpfnWndProc,
        HINSTANCE hInstance,
        LPCWSTR className,
        int cbClsExtra = 0,
        int cbWndExtra = 0,
        HICON hIcon = nullptr,
        HCURSOR hCursor = nullptr,
        HBRUSH hbrBackground = nullptr,
        LPCWSTR lpszMenuName = nullptr,
        HICON hIconSm = nullptr
    ) : hInstance(hInstance), name(className) {
        WNDCLASSEX wcex{};
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = style;
        wcex.lpfnWndProc = lpfnWndProc;
        wcex.cbClsExtra = cbClsExtra;
        wcex.cbWndExtra = cbWndExtra;
        wcex.hInstance = hInstance;
        wcex.hIcon = hIcon;
        wcex.hCursor = hCursor;
        wcex.hbrBackground = hbrBackground;
        wcex.lpszMenuName = lpszMenuName;
        wcex.lpszClassName = className;
        wcex.hIconSm = hIconSm;
        if (!RegisterClassEx(&wcex)) {
            throw std::runtime_error("Failed to register window class.");
        }
    }

    WindowClass(const WindowClass&) = delete;
    WindowClass& operator=(const WindowClass&) = delete;

    WindowClass(WindowClass&& other) noexcept
        : hInstance(other.hInstance), name(other.name) {
        other.drop();
    }

    WindowClass& operator=(WindowClass&& other) noexcept {
        if (this != &other) {
            clean();
            hInstance = other.hInstance;
            name = other.name;
            other.drop();
        }
        return *this;
    }

    ~WindowClass() {
        clean();
    }

    operator bool() const {
        return name != nullptr;
    }

    LPCWSTR get() const {
        return name;
    }

    operator LPCWSTR() const {
        return name;
    }


private:
    HINSTANCE hInstance = nullptr;
    LPCWSTR name = nullptr;

    void drop() {
        hInstance = nullptr;
        name = nullptr;
    }

    void clean() {
        if (!*this) return;
        UnregisterClass(name, hInstance);
        drop();
    }
};


std::wstring toUTF16(const std::string& source) {
    int size = MultiByteToWideChar(CP_UTF8, 0, source.c_str(), -1, nullptr, 0);
    if (size == 0) {
        throw std::runtime_error("UTF-8 to UTF-16 conversion failed");
    }
    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, source.c_str(), -1, &result[0], size);
    return result;
}


std::string toUTF8(LPCWSTR source) {
    if (!source) {
        return std::string();
    }

    int size = WideCharToMultiByte(CP_UTF8, 0, source, -1, nullptr, 0, nullptr, nullptr);
    if (size == 0) {
        throw std::runtime_error("UTF-16 to UTF-8 conversion failed");
    }
    std::string result(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, source, -1, &result[0], size, nullptr, nullptr);
    result.resize(size - 1);
    return result;
}


class WndHandle {
public:
    WndHandle() = default;

    WndHandle(HWND hWnd) : hWnd(hWnd) {}

    ~WndHandle() {
        clean();
    }

    WndHandle(const WndHandle&) = delete;
    WndHandle& operator=(const WndHandle&) = delete;

    WndHandle(WndHandle&& other) noexcept
        : hWnd(other.hWnd) {
        other.drop();
    }

    WndHandle& operator=(WndHandle&& other) noexcept {
        if (this != &other) {
            clean();
            hWnd = other.hWnd;
            other.drop();
        }
        return *this;
    }

    operator bool() const {
        return hWnd != nullptr;
    }

    HWND get() const {
        return hWnd;
    }

private:
    HWND hWnd = nullptr;

    void clean() {
        if (hWnd != nullptr) {
            DestroyWindow(hWnd);
        }
        drop();
    }

    void drop() {
        hWnd = nullptr;
    }
};



HINSTANCE getHInstance() {
    static HINSTANCE hInstance =
        reinterpret_cast<HINSTANCE>(GetModuleHandle(NULL));
    return hInstance;
}

LRESULT CALLBACK windowProc(
        HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (auto handler = reinterpret_cast<MessageProxy*>(
            GetWindowLongPtr(hWnd, GWLP_USERDATA)))
    {
        return handler->handleMessage(
            SysMessage(hWnd, msg, wParam, lParam));
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

WindowClass& getWndClass() {
    static WindowClass wc(
        CS_OWNDC,
        windowProc,
        getHInstance(),
        L"Istok",
        0, 0, nullptr,
        LoadCursor(NULL, IDC_ARROW));
    return wc;
}


class WinAPINotifierWindow {
public:
    WinAPINotifierWindow(MessageProxy& proxy)
        :wnd(makeWindow())
    {
        SetWindowLongPtr(
            wnd.get(), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&proxy));
    }

    void postQueueNotification() noexcept {
        PostMessage(wnd.get(), WM_APP_QUEUE, NULL, NULL);
    }

private:
    WndHandle wnd;

    static WndHandle makeWindow() {
        WndHandle wnd(CreateWindowEx(
            WS_EX_TOOLWINDOW,
            getWndClass(),
            L"",
            WS_POPUP,
            0, 0, 0, 0,
            NULL, NULL, getHInstance(), nullptr));
        if (!wnd) {
            throw std::runtime_error("Cannot create window");
        }
        return wnd;
    }
};


class GLWindow {
public:
    GLWindow(WindowParams params, MessageProxy& proxy)
        :wnd(makeWindow(params, proxy))
    {
        SetWindowLongPtr(
            wnd.get(), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&proxy));
    }

    void setTranslator(std::unique_ptr<WindowTranslator>&& value) {
        translator = std::move(value);
    }
    
    void removeTranslator() {
        translator.reset();
    }

    HWND getHWnd() const noexcept {
        return wnd.get();
    }

    void onClose() noexcept {
        if (!translator) {
            return;
        }
        translator->onClose();
    }

private:
    WndHandle wnd;
    std::unique_ptr<WindowTranslator> translator;

    static WndHandle makeWindow(WindowParams params, MessageProxy& proxy) {
        WndHandle wnd(CreateWindowEx(
            params.title.has_value() ? NULL : WS_EX_TOOLWINDOW,
            getWndClass(),
            toUTF16(params.title.value_or("")).c_str(),
            WS_OVERLAPPEDWINDOW, //WS_POPUP,
            params.location.left, params.location.top,
            params.location.right - params.location.left,
            params.location.bottom - params.location.top,
            NULL, NULL, getHInstance(), nullptr));
        if (!wnd) {
            throw std::runtime_error("Cannot create window");
        }
        enableTransparency(wnd);
        SetWindowLongPtr(
            wnd.get(), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&proxy));
        return wnd;
    }

    static void enableTransparency(WndHandle& wnd) {
        DWM_BLURBEHIND bb = { 0 };
        HRGN hRgn = CreateRectRgn(0, 0, -1, -1);
        bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
        bb.hRgnBlur = hRgn;
        bb.fEnable = TRUE;
        DwmEnableBlurBehindWindow(wnd.get(), &bb);
    }
};


class WinAPIWindowManager: public MessageProxy {
public:
    using Window = GLWindow;

    std::shared_ptr<Window> create(WindowParams params) {
        auto window = std::make_shared<Window>(params, *this);
        windows[window->getHWnd()] = window;
        ShowWindow(window->getHWnd(), SW_SHOW);
        return window;
    }

    void remove(std::shared_ptr<Window> window) {
        windows.erase(window->getHWnd());
    }
    
    void runMessageLoop() {
        while (true) {
            MSG msg;
            GetMessage(&msg, NULL, 0, 0);
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    void stopMessageLoop() noexcept {
        PostQuitMessage(0);
    }

    SysResult handleMessage(SysMessage message) noexcept override {
        if (!windows.contains(message.hWnd)) {
            return handleByDefault(message);
        }
        auto window = windows[message.hWnd];
        switch (message.msg) {
        case WM_CLOSE:
            window->onClose();
            return 0;
        case WM_DESTROY:
            return 0;
        default:
            return handleByDefault(message);
        }
    }

private:
    std::unordered_map<HWND, std::shared_ptr<Window>> windows;
};

} // namespace Istok::GUI::WinAPI

/*
class SysWindow;

class SysMessageHandler {
public:
    virtual void onQueue() = 0;
    virtual void onClose(SysWindow& window) = 0;
};


struct WindowParams {
    Rect<int> location;
    std::optional<std::string> title;
};


class GLWindow {
public:
    GLWindow(WinAPIMessageHandler* messageHandler)
        : wnd(makeWindow(WindowParams{}, messageHandler)), gl(wnd) {}

    GLWindow(
        const GLWindow& other,
        WindowParams params, WinAPIMessageHandler* messageHandler
    ) : wnd(makeWindow(params, messageHandler)), gl(wnd, other.gl) {}

    ~GLWindow() {
        std::cout << "-window" << std::endl << std::flush;
        setMessageHandler(wnd, nullptr);
    }
    
    GLWindow(const GLWindow&) = delete;
    GLWindow& operator=(const GLWindow&) = delete;
    GLWindow(GLWindow&&) = delete;
    GLWindow& operator=(GLWindow&& other) = delete;

    HWND getHWND() {
        return wnd.get();
    }

    void activateGL() {
        gl.activate();
    }

    void swapBuffers() {
        gl.swapBuffers();
    }

private:
    WndHandle wnd;
    GLManager gl;

    static WndHandle makeWindow(
        WindowParams params,
        WinAPIMessageHandler* messageHandler
    ) {
        std::cout << "+window" << std::endl << std::flush;
        WndHandle wnd(CreateWindowEx(
            params.title.has_value() ? NULL : WS_EX_TOOLWINDOW,
            getWndClass(),
            toUTF16(params.title.value_or("")).c_str(),
            WS_POPUP,
            params.location.left, params.location.top,
            params.location.right - params.location.left,
            params.location.bottom - params.location.top,
            NULL, NULL, getHInstance(), nullptr));
        if (!wnd) {
            throw std::runtime_error("Cannot create window");
        }
        enableTransparency(wnd);
        setMessageHandler(wnd, messageHandler);
        return wnd;
    }

    static HINSTANCE getHInstance() {
        static HINSTANCE hInstance =
            reinterpret_cast<HINSTANCE>(GetModuleHandle(NULL));
        return hInstance;
    }
    
    static WindowClass& getWndClass() {
        static WindowClass wc(
            CS_OWNDC,
            windowProc,
            getHInstance(),
            L"Istok",
            0, 0, nullptr,
            LoadCursor(NULL, IDC_ARROW));
        return wc;
    }

    static void enableTransparency(WndHandle& wnd) {
        DWM_BLURBEHIND bb = { 0 };
        HRGN hRgn = CreateRectRgn(0, 0, -1, -1);
        bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
        bb.hRgnBlur = hRgn;
        bb.fEnable = TRUE;
        DwmEnableBlurBehindWindow(wnd.get(), &bb);
    }

    static void setMessageHandler(WndHandle& wnd, WinAPIMessageHandler* value) {
        SetWindowLongPtr(
            wnd.get(), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(value));
    }

    static LRESULT CALLBACK windowProc(
            HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (WinAPIMessageHandler* handler =
            reinterpret_cast<WinAPIMessageHandler*>(
                GetWindowLongPtr(hWnd, GWLP_USERDATA)))
        {
            return handler->handleMessage(
                WinAPIMessage(hWnd, msg, wParam, lParam));
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
};


struct Color {
    u_char r;
    u_char g;
    u_char b;
    u_char a;
};


class SmartWindow {
public:
    SmartWindow(WinAPIMessageHandler* messageHandler)
        : core(messageHandler) {}

    SmartWindow(
        const SmartWindow& other,
        WindowParams params, WinAPIMessageHandler* messageHandler
    ) : core(other.core, params, messageHandler) {}

    SmartWindow(const SmartWindow&) = delete;
    SmartWindow& operator=(const SmartWindow&) = delete;
    SmartWindow(SmartWindow&&) = delete;
    SmartWindow& operator=(SmartWindow&& other) = delete;

    void postQueueNotification() {
        PostMessage(core.getHWND(), WM_APP_QUEUE, NULL, NULL);
    }

    void show() {
        ShowWindow(core.getHWND(), SW_SHOW);
    }

    void hide() {
        ShowWindow(core.getHWND(), SW_HIDE);
    }

    void paint() {
        std::cout << "gui: WM_PAINT" << std::endl << std::flush;
        core.activateGL();
        constexpr float factor = 1.0 / 255.0;
        glClearColor(
            color.r * factor,
            color.g * factor,
            color.b * factor,
            color.a * factor);
        glClear(GL_COLOR_BUFFER_BIT);
        core.swapBuffers();
        finishPaint();
    }


    SysResult hitTest(WinAPIMessage message) {
        POINT point = {
            GET_X_LPARAM(message.lParam),
            GET_Y_LPARAM(message.lParam)};
        RECT rect;
        GetWindowRect(core.getHWND(), &rect);
        bool lb = point.x - rect.left < 4;
        bool rb = rect.right - point.x < 4;
        bool tb = point.y - rect.top < 4;
        bool bb = rect.bottom - point.y < 4;
        if (lb && tb) return HTTOPLEFT;
        if (rb && tb) return HTTOPRIGHT;
        if (lb && bb) return HTBOTTOMLEFT;
        if (rb && bb) return HTBOTTOMRIGHT;
        if (lb) return HTLEFT;
        if (rb) return HTRIGHT;
        if (tb) return HTTOP;
        if (bb) return HTBOTTOM;
        if (point.y - rect.top < 32) return HTCAPTION;
        return HTCLIENT;
    }


    void setColor(Color color) {
        this->color = color;
    }


private:
    GLWindow core;
    Color color;

    void finishPaint() {
        PAINTSTRUCT ps;
        BeginPaint(core.getHWND(), &ps);
        EndPaint(core.getHWND(), &ps);
    }
};


class SysWindow : public WinAPIMessageHandler {
public:
    // Create window with default parameters
    SysWindow(SysMessageHandler& messageHandler)
        : core(this), messageHandler(&messageHandler) {}

    // Create duplicate with different parameters
    SysWindow(const SysWindow& sample, WindowParams params)
        : core(sample.core, params, this), messageHandler(sample.messageHandler) {}
    
    SysWindow(const SysWindow&) = delete;
    SysWindow& operator=(const SysWindow&) = delete;
    SysWindow(SysWindow&& other) = delete;
    SysWindow& operator=(SysWindow&& other) = delete;

    SysResult handleMessage(WinAPIMessage message) override {
        switch (message.msg) {
        case WM_CLOSE:
            messageHandler->onClose(*this);
            return 0;
        case WM_DESTROY:
            return 0;
        case WM_APP_QUEUE:
            messageHandler->onQueue();
            return 0;
        case WM_PAINT:
            core.paint();
            return 0;
        case WM_NCHITTEST:
            return core.hitTest(message);
        default:
            return defaultWinAPIHandler(message);
        }
    }

    void postQueueNotification() {
        core.postQueueNotification();
    }

    void show() {
        core.show();
    }

    void hide() {
        core.hide();
    }
    
    void setColor(Color color) {
        core.setColor(color);
    }

private:
    SmartWindow core;
    SysMessageHandler* messageHandler;
};
*/

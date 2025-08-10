// window.hpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <cassert>
#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <stdexcept>
#include <iostream>
#include <optional>

#include <gui/core/tools.hpp>
#include <gui/core/messages.hpp>

#include "gl.hpp"

constexpr UINT WM_APP_QUEUE = WM_APP + 1;


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


class WndHandler {
public:
    WndHandler() = default;

    WndHandler(HWND hWnd) : hWnd(hWnd) {}

    ~WndHandler() {
        clean();
    }

    WndHandler(const WndHandler&) = delete;
    WndHandler& operator=(const WndHandler&) = delete;

    WndHandler(WndHandler&& other) noexcept
        : hWnd(other.hWnd) {
        other.drop();
    }

    WndHandler& operator=(WndHandler&& other) noexcept {
        if (this != &other) {
            clean();
            hWnd = other.hWnd;
            other.drop();
        }
        return *this;
    }

    void clean() {
        if (hWnd != nullptr) {
            DestroyWindow(hWnd);
        }
        drop();
    }

    operator bool() const {
        return hWnd != nullptr;
    }

    HWND get() const {
        return hWnd;
    }

private:
    HWND hWnd = nullptr;

    void drop() {
        hWnd = nullptr;
    }
};


class DCHandler {
public:
    DCHandler() = default;
    
    DCHandler(HDC hDC) : hDC(hDC) {}

    ~DCHandler() {
        clean();
    }

    DCHandler(const DCHandler&) = delete;
    DCHandler& operator=(const DCHandler&) = delete;

    DCHandler(DCHandler&& other) noexcept
        : hDC(other.hDC) {
        other.drop();
    }

    DCHandler& operator=(DCHandler&& other) noexcept {
        if (this != &other) {
            clean();
            hDC = other.hDC;
            other.drop();
        }
        return *this;
    }

    void clean() {
        if (hDC != nullptr) {
            ReleaseDC(WindowFromDC(hDC), hDC);
        }
        drop();
    }

    operator bool() const {
        return hDC != nullptr;
    }

    HDC get() const {
        return hDC;
    }

private:
    HDC hDC = nullptr;

    void drop() {
        hDC = nullptr;
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


struct WinAPIMessage {
    HWND hWnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
};


using SysResult = LRESULT;


class WinAPIMessageHandler {
public:
    virtual SysResult handleMessage(WinAPIMessage message) = 0;
};


SysResult defaultWinAPIHandler(WinAPIMessage message) {
    return DefWindowProc(
        message.hWnd,
        message.msg,
        message.wParam,
        message.lParam
    );
}


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


class GLManager {
public:
    GLManager(WndHandler& wnd)
        : dc(makeDC(wnd)), gl(std::make_shared<ModernGLContext>(dc.get())) {}

    GLManager(WndHandler& wnd, const GLManager& other)
        : dc(makeDC(wnd)), gl(other.gl) {}

    GLManager(const GLManager&) = delete;
    GLManager& operator=(const GLManager&) = delete;
    GLManager(const GLManager&&) = delete;
    GLManager& operator=(const GLManager&&) = delete;

    void activate() {
        wglMakeCurrent(dc.get(), gl->getGL());
    }

    void swapBuffers() {
        SwapBuffers(dc.get());
    }

private:
    DCHandler dc;
    std::shared_ptr<ModernGLContext> gl;

    static DCHandler makeDC(WndHandler& wnd) {
        DCHandler dc(GetWindowDC(wnd.get()));
        if (!dc) {
            throw std::runtime_error("Cannot get window DC");
        }
        setPixelFormat(dc);
        return dc;
    }

    static void setPixelFormat(DCHandler& dc) {
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

        int pfi = ChoosePixelFormat(dc.get(), &pfd);
        if (!pfi) {
            throw std::runtime_error("Failed to choose pixel format");
        }
        if (!SetPixelFormat(dc.get(), pfi, &pfd)) {
            throw std::runtime_error("Failed to set pixel format");
        }
    }
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
    WndHandler wnd;
    GLManager gl;

    static WndHandler makeWindow(
        WindowParams params,
        WinAPIMessageHandler* messageHandler
    ) {
        std::cout << "+window" << std::endl << std::flush;
        WndHandler wnd(CreateWindowEx(
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

    static void enableTransparency(WndHandler& wnd) {
        DWM_BLURBEHIND bb = { 0 };
        HRGN hRgn = CreateRectRgn(0, 0, -1, -1);
        bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
        bb.hRgnBlur = hRgn;
        bb.fEnable = TRUE;
        DwmEnableBlurBehindWindow(wnd.get(), &bb);
    }

    static void setMessageHandler(WndHandler& wnd, WinAPIMessageHandler* value) {
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

    void activateGL() {
        core.activateGL();
    }

    void swapBuffers() {
        core.swapBuffers();
    }

    void postQueueNotification() {
        PostMessage(core.getHWND(), WM_APP_QUEUE, NULL, NULL);
    }

    void show() {
        ShowWindow(core.getHWND(), SW_SHOW);
    }

    void hide() {
        ShowWindow(core.getHWND(), SW_HIDE);
    }

    void finishPaint() {
        PAINTSTRUCT ps;
        BeginPaint(core.getHWND(), &ps);
        EndPaint(core.getHWND(), &ps);
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


private:
    GLWindow core;
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
            onPaint();
            return 0;
        case WM_NCHITTEST:
            return core.hitTest(message);
        default:
            return defaultWinAPIHandler(message);
        }
    }

    void activateGL() {
        core.activateGL();
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
    

private:
    SmartWindow core;
    SysMessageHandler* messageHandler;

    void onPaint() {
        std::cout << "gui: WM_PAINT" << std::endl << std::flush;
        core.activateGL();
        glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        core.swapBuffers();
        core.finishPaint();
    }
};

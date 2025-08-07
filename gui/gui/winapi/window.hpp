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


struct SysMessage {
    HWND hWnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
};

using SysResult = LRESULT;


class SysMessageHandler {
public:
    virtual SysResult handleSysMessage(SysMessage message) = 0;
};


SysResult handleSysMessageByDefault(SysMessage message) {
    return DefWindowProc(
        message.hWnd,
        message.msg,
        message.wParam,
        message.lParam);
}


struct SysWindowParams {
    Rect<int> location;
    std::optional<std::string> title;
};


class DCWindow {
public:
    DCWindow(SysWindowParams params, SysMessageHandler* messageHandler) {
        wnd = CreateWindowEx(
            params.title.has_value() ? NULL : WS_EX_TOOLWINDOW,
            getWndClass(),
            toUTF16(params.title.value_or("")).c_str(),
            WS_OVERLAPPEDWINDOW, //TODO: change to WS_POPUP for custom decorations
            params.location.left, params.location.top,
            params.location.right - params.location.left,
            params.location.bottom - params.location.top,
            NULL, NULL, getHInstance(), nullptr);
        if (!wnd) {
            throw std::runtime_error("Cannot create window");
        }
        dc = GetWindowDC(wnd.get());
        if (!dc) {
            throw std::runtime_error("Cannot get window DC");
        }
        setPixelFormat();
        enableTransparency();
        setMessageHandler(messageHandler);
    }

    ~DCWindow() {
        clean();
    }
    
    DCWindow(const DCWindow&) = delete;
    DCWindow& operator=(const DCWindow&) = delete;

    // Disabled to prevent message handler invalidation
    DCWindow(DCWindow&&) = delete;
    DCWindow& operator=(DCWindow&& other) = delete;
    
    // Move and change message handler
    DCWindow(DCWindow&& other, SysMessageHandler* messageHandler)
        : wnd(std::move(other.wnd)), dc(std::move(dc))
    {
        setMessageHandler(messageHandler);
    }

    void replace(DCWindow&& other, SysMessageHandler* messageHandler) {
        if (this == &other) {
            return;
        }
        clean();
        wnd = std::move(other.wnd);
        dc = std::move(other.dc);
        setMessageHandler(messageHandler);
    }


    operator bool() const {
        return wnd && dc;
    }

    HDC getDC() {
        return dc.get();
    }

    HWND getHWND() {
        return wnd.get();
    }

private:
    WndHandler wnd;
    DCHandler dc;

    void clean() {
        setMessageHandler(nullptr);
        dc.clean();
        wnd.clean();
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

    void setPixelFormat() {
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

    void enableTransparency() {
        DWM_BLURBEHIND bb = { 0 };
        HRGN hRgn = CreateRectRgn(0, 0, -1, -1);
        bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
        bb.hRgnBlur = hRgn;
        bb.fEnable = TRUE;
        DwmEnableBlurBehindWindow(wnd.get(), &bb);
    }

    void setMessageHandler(SysMessageHandler* value) {
        SetWindowLongPtr(
            wnd.get(), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(value));
    }

    static LRESULT CALLBACK windowProc(
            HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (SysMessageHandler* handler =
            reinterpret_cast<SysMessageHandler*>(
                GetWindowLongPtr(hWnd, GWLP_USERDATA)))
        {
            return handler->handleSysMessage(
                SysMessage(hWnd, msg, wParam, lParam));
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
};


class SysWindow : public SysMessageHandler {
public:
    SysWindow(SysWindowParams params, SysMessageHandler& messageHandler)
        : dcWindow(params, this), messageHandler(&messageHandler) {}
    
    SysWindow(const SysWindow&) = delete;
    SysWindow& operator=(const SysWindow&) = delete;
    
    SysWindow(SysWindow&& other)
        : dcWindow(std::move(other.dcWindow), this),
        messageHandler(other.messageHandler) {}

    SysWindow& operator=(SysWindow&& other) {
        if (this != &other) {
            dcWindow.replace(std::move(other.dcWindow), this);
            messageHandler = other.messageHandler;
        }
        return *this;
    }

    SysResult handleSysMessage(SysMessage message) override {
        switch (message.msg) {
        case WM_DESTROY:
            return 0;
        default:
            return messageHandler->handleSysMessage(message);
        }
    }

    void postMessage(UINT msg) {
        PostMessage(dcWindow.getHWND(), msg, NULL, NULL);
    }

    void show() {
        ShowWindow(dcWindow.getHWND(), SW_SHOW);
    }

    void hide() {
        ShowWindow(dcWindow.getHWND(), SW_HIDE);
    }

private:
    DCWindow dcWindow;
    SysMessageHandler* messageHandler;
};

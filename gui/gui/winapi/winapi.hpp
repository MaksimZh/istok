// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <tools/exchange.hpp>
#include "window.hpp"

#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>

#include <iostream>
#include <memory>
#include <unordered_map>

namespace Istok::GUI::WinAPI {


HINSTANCE getHInstance() {
    static HINSTANCE hInstance =
        reinterpret_cast<HINSTANCE>(GetModuleHandle(NULL));
    return hInstance;
}


class WindowClass {
public:
    WindowClass() = default;

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


class SysMessageHandler {
public:
    virtual ~SysMessageHandler() = default;
    virtual LRESULT handleMessage(
        HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept = 0;
};


class BasicWindow {
public:
    BasicWindow(const WindowParams& params, SysMessageHandler* handler) {
        hWnd = CreateWindowEx(
            params.title.has_value() ? NULL : WS_EX_TOOLWINDOW,
            getWindowClass(),
            toUTF16(params.title.value_or("")).c_str(),
            WS_POPUP,
            params.location.left, params.location.top,
            params.location.right - params.location.left,
            params.location.bottom - params.location.top,
            NULL, NULL, getHInstance(), nullptr);
        if (!hWnd) {
            throw std::runtime_error("Cannot create window");
        }
        SetWindowLongPtr(
            hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(handler));
    }

    ~BasicWindow() noexcept {
        if (hWnd) {
            SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);
            DestroyWindow(hWnd);
        }
    }

    HWND getHandle() const noexcept {
        return hWnd;
    }

private:
    HWND hWnd = nullptr;

    static LPCWSTR getWindowClass() {
        static WindowClass wc(windowProc, L"Istok");
        return wc.get();
    }

    static LRESULT CALLBACK windowProc(
        HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
    {
        if (auto handler = reinterpret_cast<SysMessageHandler*>(
                GetWindowLongPtr(hWnd, GWLP_USERDATA)))
        {
            return handler->handleMessage(hWnd, msg, wParam, lParam);
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    static std::wstring toUTF16(const std::string& source) {
        int size = MultiByteToWideChar(
            CP_UTF8, 0, source.c_str(), -1, nullptr, 0);
        if (size == 0) {
            throw std::runtime_error("UTF-8 to UTF-16 conversion failed");
        }
        std::wstring result(size, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, source.c_str(), -1, &result[0], size);
        return result;
    }
};


class HWndWindow: public SysMessageHandler {
public:
    HWndWindow(const WindowParams& params, MessageHandler& handler)
    : window(params, this), handler(handler) {
        enableTransparency();
        ShowWindow(window.getHandle(), SW_SHOW);
    }

    HWND getHandle() const noexcept {
        return window.getHandle();
    }

    void setHandler(Tools::HandlerChain<LRESULT, WindowMessage>::Handler handler) {
        handlerChain.add(handler);
    }

private:
    BasicWindow window;
    MessageHandler& handler;
    Tools::HandlerChain<LRESULT, WindowMessage> handlerChain;

    LRESULT handleMessage(
        HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        if (auto r = handlerChain(WindowMessage(hWnd, msg, wParam, lParam))) {
            return r.value();
        }
        switch (msg) {
        case WM_PAINT: {
            handler.onPaint();
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            return 0;
        }
        case WM_NCHITTEST: {
            RECT rect;
            GetWindowRect(hWnd, &rect);
            Position<int> position(
                GET_X_LPARAM(lParam) - rect.left,
                GET_Y_LPARAM(lParam) - rect.top);
            switch (handler.onAreaTest(position)) {
            case WindowArea::hole: return HTTRANSPARENT;
            case WindowArea::client: return HTCLIENT;
            case WindowArea::moving: return HTCAPTION;
            case WindowArea::sizingTL: return HTTOPLEFT;
            case WindowArea::sizingT: return HTTOP;
            case WindowArea::sizingTR: return HTTOPRIGHT;
            case WindowArea::sizingR: return HTRIGHT;
            case WindowArea::sizingBR: return HTBOTTOMRIGHT;
            case WindowArea::sizingB: return HTBOTTOM;
            case WindowArea::sizingBL: return HTBOTTOMLEFT;
            case WindowArea::sizingL: return HTLEFT;
            default: return HTCLIENT;
            }
        }
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    void enableTransparency() {
        DWM_BLURBEHIND bb = { 0 };
        HRGN hRgn = CreateRectRgn(0, 0, -1, -1);
        bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
        bb.hRgnBlur = hRgn;
        bb.fEnable = TRUE;
        DwmEnableBlurBehindWindow(window.getHandle(), &bb);
    }
};


} // namespace Istok::GUI::WinAPI

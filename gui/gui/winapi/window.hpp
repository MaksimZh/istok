// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <cassert>
#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <stdexcept>
#include <iostream>
#include <semaphore>
#include <mutex>
#include <queue>

#include <gui/core/tools.hpp>
#include <gui/core/messages.hpp>

constexpr UINT WM_APP_QUEUE = WM_APP + 1;

LRESULT CALLBACK windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


class Notifier {
public:
    Notifier(HWND target) : target(target) {}
    
    void operator()() {
        PostMessage(target, WM_APP_QUEUE, NULL, NULL);
    }
private:
    HWND target;
};

template <typename T>
class GUIMessageQueue {
public:
    GUIMessageQueue() = default;
    GUIMessageQueue(const GUIMessageQueue&) = delete;
    GUIMessageQueue& operator=(const GUIMessageQueue&) = delete;
    GUIMessageQueue(GUIMessageQueue&&) = delete;
    GUIMessageQueue& operator=(GUIMessageQueue&&) = delete;
    
    bool ready() const {
        return container != nullptr;
    }
    
    void init(HWND hWnd) {
        if (ready()) {
            return;
        }
        container = std::make_unique<Queue>(Notifier(hWnd));
    }

    bool empty() {
        assert(ready());
        return container->empty();
    }
    
    void push(T&& value) {
        assert(ready());
        container->push(std::move(value));
    }
    
    T take() {
        assert(ready());
        return container->take();
    }

private:
    using Queue = Istok::GUI::SyncNotifyingQueue<T, Notifier>;
    std::unique_ptr<Queue> container;
};


class WndClassHandler {
public:
    WndClassHandler() = default;

    WndClassHandler(
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

    WndClassHandler(const WndClassHandler&) = delete;
    WndClassHandler& operator=(const WndClassHandler&) = delete;

    WndClassHandler(WndClassHandler&& other) noexcept
        : hInstance(other.hInstance), name(other.name) {
        other.drop();
    }

    WndClassHandler& operator=(WndClassHandler&& other) noexcept {
        if (this != &other) {
            clean();
            hInstance = other.hInstance;
            name = other.name;
            other.drop();
        }
        return *this;
    }

    ~WndClassHandler() {
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

    WndHandler(
        DWORD dwExStyle,
        LPCWSTR lpClassName,
        LPCWSTR lpWindowName,
        DWORD dwStyle,
        int x,
        int y,
        int nWidth,
        int nHeight,
        HWND hWndParent,
        HMENU hMenu,
        HINSTANCE hInstance,
        LPVOID lpParam
    ) : hWnd(CreateWindowEx(
                dwExStyle, lpClassName, lpWindowName, dwStyle,
                x, y, nWidth, nHeight,
                hWndParent, hMenu, hInstance, lpParam)) {
        if (!hWnd) {
            throw std::runtime_error("Failed to create window.");
        }
    }

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

    operator bool() const {
        return hWnd != nullptr;
    }

    HWND get() const {
        return hWnd;
    }

    operator HWND() const {
        return hWnd;
    }


private:
    HWND hWnd = nullptr;

    void drop() {
        hWnd = nullptr;
    }

    void clean() {
        if (!*this) return;
        DestroyWindow(hWnd);
        drop();
    }
};


class DCHandler {
public:
    DCHandler() = default;
    
    DCHandler(HWND hWnd) : hDC(GetWindowDC(hWnd)) {
        if (!hDC) {
            throw std::runtime_error("Failed to get window device context");
        }
    }

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

    operator bool() const {
        return hDC != nullptr;
    }

    HDC get() const {
        return hDC;
    }

    operator HDC() const {
        return hDC;
    }

private:
    HDC hDC = nullptr;

    void drop() {
        hDC = nullptr;
    }

    void clean() {
        if (*this) {
            ReleaseDC(WindowFromDC(hDC), hDC);
        }
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


class SysWindow {
public:
    SysWindow() = default;
    
    SysWindow(
        const std::string& title, Position<int> position, Size<int> size,
        bool isTool,
        Istok::GUI::SyncWaitingQueue<bool>& outQueue,
        GUIMessageQueue<int>& inQueue)
        : wnd(
            isTool ? WS_EX_TOOLWINDOW : NULL,
            getWndClass(),
            toUTF16(title).c_str(),
            WS_POPUP,
            position.x, position.y,
            size.width, size.height,
            NULL, NULL, getHInstance(), this),
        dc(wnd), outQueue(&outQueue), inQueue(&inQueue)
    {
        if (!inQueue.ready()) {
            inQueue.init(wnd.get());
        }
        setPixelFormat();
        enableTransparency();
    }
    
    SysWindow(const SysWindow&) = delete;
    SysWindow& operator=(const SysWindow&) = delete;

    SysWindow(SysWindow&& other) noexcept
        : wnd(std::move(other.wnd)), dc(std::move(other.dc))
    {}

    SysWindow& operator=(SysWindow&& other) noexcept {
        wnd = std::move(other.wnd);
        dc = std::move(other.dc);
        return *this;
    }

    operator bool() const {
        return wnd && dc;
    }

    void show() {
        ShowWindow(wnd, SW_SHOW);
    }

    HDC getDC() {
        return dc.get();
    }

    HWND getHWND() {
        return wnd.get();
    }

    
    LRESULT handleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_NCHITTEST: {
            POINT point = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            RECT rect;
            GetWindowRect(wnd, &rect);
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
        case WM_MOVING:
            assert(outQueue);
            outQueue->push(true);
            return TRUE;
        case WM_APP_QUEUE:
            assert(inQueue);
            assert(!inQueue->empty());
            std::cout << inQueue->take() << " " << wnd.get() << std::endl;
            return 0;
        default:
            return DefWindowProc(wnd, msg, wParam, lParam);
        }
    }

private:

    static HINSTANCE getHInstance() {
        static HINSTANCE hInstance =
            reinterpret_cast<HINSTANCE>(GetModuleHandle(NULL));
        return hInstance;
    }
    
    static WndClassHandler& getWndClass() {
        static WndClassHandler wc(
            CS_OWNDC,
            windowProc,
            getHInstance(),
            L"Istok",
            0, 0, nullptr,
            LoadCursor(NULL, IDC_ARROW));
        return wc;
    }

    WndHandler wnd;
    DCHandler dc;
    Istok::GUI::SyncWaitingQueue<bool>* outQueue;
    GUIMessageQueue<int>* inQueue;

    void setPixelFormat() {
        PIXELFORMATDESCRIPTOR pfd = {};
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SUPPORT_COMPOSITION;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cAlphaBits = 8;
        pfd.cDepthBits = 24;
        pfd.cStencilBits = 8;
        pfd.iLayerType = PFD_MAIN_PLANE;

        int pfi = ChoosePixelFormat(dc, &pfd);
        if (!pfi) {
            throw std::runtime_error("Failed to choose pixel format");
        }
        if (!SetPixelFormat(dc, pfi, &pfd)) {
            throw std::runtime_error("Failed to set pixel format");
        }
    }

    void enableTransparency() {
        DWM_BLURBEHIND bb = { 0 };
        HRGN hRgn = CreateRectRgn(0, 0, -1, -1);
        bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
        bb.hRgnBlur = hRgn;
        bb.fEnable = TRUE;
        DwmEnableBlurBehindWindow(wnd, &bb);
    }
};


LRESULT CALLBACK windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (SysWindow* sw = reinterpret_cast<SysWindow*>(
        GetWindowLongPtr(hWnd, GWLP_USERDATA)))
    {
        return sw->handleMessage(msg, wParam, lParam);
    }

    if (msg != WM_CREATE) {
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    CREATESTRUCT* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
    SysWindow* sw = static_cast<SysWindow*>(createStruct->lpCreateParams);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(sw));
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

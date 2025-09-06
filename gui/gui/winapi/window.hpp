// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "platform.hpp"

#include <windows.h>
#include <windowsx.h>

#include <memory>

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


class MessageHandler {
public:
    virtual void onClose() noexcept = 0;
    virtual void onPaint() noexcept = 0;
    virtual WindowArea onAreaTest(Position<int> position) noexcept = 0;
};


class DCHandle {
public:
    DCHandle(HDC hDC) : hDC(hDC) {}
    
    DCHandle(HWND hWnd) : hDC(GetDC(hWnd)) {
        if (hDC == nullptr) {
            throw std::runtime_error("Failed to get window DC");
        }
    }

    DCHandle(const DCHandle&) = delete;
    DCHandle& operator=(const DCHandle&) = delete;
    
    DCHandle(DCHandle&& other) {
        hDC = other.hDC;
        other.drop();
    }

    DCHandle& operator=(DCHandle&& other) {
        if (this != &other) {
            clean();
            hDC = other.hDC;
            other.drop();
        }
        return *this;
    }

    ~DCHandle() {
        clean();
    }

    operator bool() const noexcept {
        return hDC != nullptr;
    }

    HDC get() const noexcept {
        return hDC;
    }

private:
    HDC hDC;

    void drop() {
        hDC = nullptr;
    }

    void clean() {
        if (hDC) {
            ReleaseDC(WindowFromDC(hDC), hDC);
        }
    }
};


class HWndWindow {
public:
    HWndWindow(const WindowParams& params, MessageHandler& handler)
    : handler(handler) {
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
            hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        ShowWindow(hWnd, SW_SHOW);
    }

    ~HWndWindow() noexcept {
        if (hWnd) {
            DestroyWindow(hWnd);
        }
    }

    struct SysContext {
        HWND hWnd;
    };

    SysContext sysContext() const noexcept {
        return SysContext(hWnd);
    }

private:
    MessageHandler& handler;
    HWND hWnd = nullptr;

    static LPCWSTR getWindowClass() {
        static WindowClass wc(windowProc, L"Istok");
        return wc.get();
    }

    static LRESULT CALLBACK windowProc(
        HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
    {
        if (auto handler = reinterpret_cast<HWndWindow*>(
                GetWindowLongPtr(hWnd, GWLP_USERDATA)))
        {
            return handler->handleMessage(hWnd, msg, wParam, lParam);
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    LRESULT handleMessage(
        HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
    {
        switch (msg) {
        case WM_CLOSE:
            handler.onClose();
            return 0;
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


template <typename Renderer>
class WindowData {
public:
    void setRenderer(std::unique_ptr<Renderer>&& renderer) {
        this->renderer = std::move(renderer);
    }
    
    Renderer& getRenderer() {
        if (renderer == nullptr) {
            throw std::runtime_error("Renderer not attached");
        }
        return *renderer;
    }

    void setAreaTester(std::unique_ptr<WindowAreaTester>&& tester) {
        this->areaTester = std::move(tester);
    }

    WindowArea testArea(Position<int> position) const noexcept {
        if (!areaTester) {
            return WindowArea::client;
        }
        return areaTester->testWindowArea(position);
    }

private:
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<WindowAreaTester> areaTester;
};


template <typename SysWindow, typename Renderer>
class WindowCore {
public:
    WindowCore(const WindowParams& params, MessageHandler& handler)
    : window(params, handler) {}

    void setRenderer(std::unique_ptr<Renderer>&& renderer) {
        if (!renderer) {
            throw std::runtime_error("No renderer provided");
        }
        renderer->prepare(window);
        data.setRenderer(std::move(renderer));
    }

    void loadScene(std::unique_ptr<typename Renderer::Scene>&& scene) {
        data.getRenderer().loadScene(std::move(scene));
    }
    
    void draw() {
        data.getRenderer().draw(window);
    }


    void setAreaTester(std::unique_ptr<WindowAreaTester>&& tester) {
        data.setAreaTester(std::move(tester));
    }

    WindowArea testArea(Position<int> position) const noexcept {
        return data.testArea(position);
    }

private:
    SysWindow window;
    WindowData<Renderer> data;
};


template <typename SysWindow, typename Renderer_>
class Window: public MessageHandler {
public:
    using Renderer = Renderer_;
    
    Window(const WindowParams& params, EventHandler<Window>& handler)
    : core(params, *this), handler(handler) {}

    void onClose() noexcept override {
        handler.onClose(this);
    }

    void onPaint() noexcept override {
        try {
            core.draw();
        } catch(...) {
            handler.onException(std::current_exception());
        }
    }

    void setRenderer(std::unique_ptr<Renderer>&& renderer) {
        core.setRenderer(std::move(renderer));
    }

    void loadScene(std::unique_ptr<typename Renderer::Scene>&& scene) {
        core.loadScene(std::move(scene));
    }


    void setAreaTester(std::unique_ptr<WindowAreaTester>&& tester) {
        core.setAreaTester(std::move(tester));
    }

    WindowArea onAreaTest(Position<int> position) noexcept override {
        return core.testArea(position);
    }

private:
    WindowCore<SysWindow, Renderer> core;
    EventHandler<Window>& handler;
};


} // namespace Istok::GUI::WinAPI

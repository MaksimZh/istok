// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "definitions.hpp"
#include <tools/queue.hpp>
#include <tools/helpers.hpp>
#include <windows.h>
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
        : name(className)
    {
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


std::wstring toUTF16(const std::string& source) {
    int size = MultiByteToWideChar(CP_UTF8, 0, source.c_str(), -1, nullptr, 0);
    if (size == 0) {
        throw std::runtime_error("UTF-8 to UTF-16 conversion failed");
    }
    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, source.c_str(), -1, &result[0], size);
    return result;
}


template <typename Window>
class WindowMessageHandler {
public:
    virtual void onClose(Window* sender) noexcept = 0;
};


template <typename Renderer>
class Window {
public:
    using Scene = Renderer::Scene;

    Window(WindowParams params, WindowMessageHandler<Window>& handler)
        : handler(handler)
    {
        hWnd = CreateWindowEx(
            params.title.has_value() ? NULL : WS_EX_TOOLWINDOW,
            getWindowClass(),
            toUTF16(params.title.value_or("")).c_str(),
            WS_OVERLAPPEDWINDOW, //WS_POPUP,
            params.location.left, params.location.top,
            params.location.right - params.location.left,
            params.location.bottom - params.location.top,
            NULL, NULL, getHInstance(), nullptr);
        if (!hWnd) {
            throw std::runtime_error("Cannot create window");
        }
        SetWindowLongPtr(
            hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    }

    ~Window() noexcept {
        if (hWnd) {
            DestroyWindow(hWnd);
        }
    }
    
    HWND getHWnd() const noexcept {
        return hWnd;
    }

    void loadScene(std::unique_ptr<Scene>&& scene) {
        this->scene = std::move(scene);
    }

private:
    WindowMessageHandler<Window>& handler;
    HWND hWnd = nullptr;
    std::unique_ptr<Scene> scene;

    static LPCWSTR getWindowClass() {
        static WindowClass wc(windowProc, L"Istok");
        return wc.get();
    }

    static LRESULT CALLBACK windowProc(
        HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (auto handler = reinterpret_cast<Window*>(
                GetWindowLongPtr(hWnd, GWLP_USERDATA)))
        {
            return handler->handleMessage(hWnd, msg, wParam, lParam);
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    LRESULT handleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (msg == WM_CLOSE) {
            handler.onClose(this);
            return 0;
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
};


template <typename ID_, typename Renderer>
class Platform: public WindowMessageHandler<Window<Renderer>> {
public:
    using ID = ID_;
    using Scene = Renderer::Scene;
    using Window = Window<Renderer>;

    Platform() {}

    PlatformEvent<ID> getMessage() noexcept {
        while (true) {
            if (!outQueue.empty()) {
                return outQueue.take();
            }
            MSG msg;
            GetMessage(&msg, NULL, 0, 0);
            if (msg.message == WM_QUIT) {
                return Event::PlatformShutdown{};
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void createWindow(ID id, WindowParams params) {
        auto window = std::make_unique<Window>(params, *this);
        ShowWindow(window->getHWnd(), SW_SHOW);
        identifiers[window.get()] = id;
        windows[id] = std::move(window);
    }

    void destroyWindow(ID id) {
        Window* window = windows[id].get();
        windows.erase(id);
        identifiers.erase(window);
    }

    void loadScene(ID windowID, std::unique_ptr<Scene>&& scene) {
        windows[windowID]->loadScene(std::move(scene));
    }

    void onClose(Window* sender) noexcept override {
        try {
            outQueue.push(Event::WindowClose(identifiers.at(sender)));
        } catch(...) {
            outQueue.push(Event::PlatformException(std::current_exception()));
        }
    }
    
private:
    Tools::SimpleQueue<PlatformEvent<ID>> outQueue;
    std::unordered_map<ID, std::unique_ptr<Window>, Tools::hash<ID>> windows;
    std::unordered_map<Window*, ID> identifiers;
};

}

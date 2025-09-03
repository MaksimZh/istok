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


class MessageHandler {
public:
    virtual void onClose() noexcept = 0;
    virtual void onPaint() noexcept = 0;
};


class HWndWindow {
public:
    HWndWindow(WindowParams params, MessageHandler& handler)
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

    ~HWndWindow() noexcept {
        if (hWnd) {
            DestroyWindow(hWnd);
        }
    }

    void show() {
        ShowWindow(hWnd, SW_SHOW);
    }

private:
    MessageHandler& handler;
    HWND hWnd = nullptr;

    static LPCWSTR getWindowClass() {
        static WindowClass wc(windowProc, L"Istok");
        return wc.get();
    }

    static LRESULT CALLBACK windowProc(
        HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (auto handler = reinterpret_cast<HWndWindow*>(
                GetWindowLongPtr(hWnd, GWLP_USERDATA)))
        {
            return handler->handleMessage(hWnd, msg, wParam, lParam);
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    LRESULT handleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_CLOSE:
            handler.onClose();
            return 0;
        case WM_PAINT:
            handler.onPaint();
            return 0;
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


template <typename SysWindow, typename Renderer>
class GraphicWindow {
public:
    GraphicWindow(
        WindowParams params, MessageHandler& handler,
        std::shared_ptr<Renderer> globalRenderer
    ) :
        window(params, handler),
        renderer(globalRenderer->prepareWindow(window))
    {}

    void show() {
        window.show();
    }

    void loadScene(std::unique_ptr<typename Renderer::Scene>&& scene) {
        renderer.loadScene(std::move(scene));
    }

    void draw() {
        renderer.draw();
    }

private:
    SysWindow window;
    Renderer::WindowRenderer renderer;
};


template <typename SysWindow, typename Renderer>
class WindowCore {
public:
    WindowCore(
        WindowParams params, MessageHandler& handler,
        std::shared_ptr<Renderer> renderer
    ) : window(params, handler, renderer) {}

    void show() {
        window.show();
    }

    void loadScene(std::unique_ptr<typename Renderer::Scene>&& scene) {
        window.loadScene(std::move(scene));
    }

    void draw() {
        window.draw();
    }

private:
    GraphicWindow<SysWindow, Renderer> window;
};


template <typename Window>
class EventHandler {
public:
    virtual void onException(std::exception_ptr exception) noexcept = 0;
    virtual void onClose(Window* sender) noexcept = 0;
};


template <typename SysWindow, typename Renderer>
class Window: public MessageHandler {
public:
    Window(
        WindowParams params, EventHandler<Window>& handler,
        std::shared_ptr<Renderer> renderer
    ) : core(params, *this, renderer), handler(handler) {}

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

    void show() {
        core.show();
    }

    void loadScene(std::unique_ptr<typename Renderer::Scene>&& scene) {
        core.loadScene(std::move(scene));
    }

private:
    WindowCore<SysWindow, Renderer> core;
    EventHandler<Window>& handler;
};


template <typename ID, typename Window>
class WindowMap {
public:
    void insert(ID id, std::unique_ptr<Window> window) {
        if (identifiers.contains(window.get()) || windows.contains(id)) {
            throw std::runtime_error("Window overwrite");
        }
        identifiers[window.get()] = id;
        windows[id] = std::move(window);
    }

    void erase(ID id) {
        if (!windows.contains(id)) {
            throw std::runtime_error("Window not found");
        }
        Window* window = windows[id].get();
        windows.erase(id);
        assert(identifiers.contains(window));
        identifiers.erase(window);
    }

    ID getID(Window* window) {
        if (!identifiers.contains(window)) {
            throw std::runtime_error("Window not found");
        }
        return identifiers[window];
    }

    Window& getWindow(ID id) {
        if (!windows.contains(id)) {
            throw std::runtime_error("Window not found");
        }
        return *windows[id];
    }

private:
    std::unordered_map<ID, std::unique_ptr<Window>, Tools::hash<ID>> windows;
    std::unordered_map<Window*, ID> identifiers;
};


template <typename Window, typename Renderer>
class WindowFactory {
public:
    WindowFactory(
        EventHandler<Window>& handler, std::shared_ptr<Renderer> renderer
    ) : handler(handler), renderer(renderer) {}

    std::unique_ptr<Window> create(WindowParams params) {
        return std::make_unique<Window>(params, handler, renderer);
    }

private:
    EventHandler<Window>& handler;
    std::shared_ptr<Renderer> renderer;
};


template <typename ID, typename SysWindow, typename Renderer>
class WindowManager {
public:
    using Window = Window<SysWindow, Renderer>;

    WindowManager(WindowFactory<Window, Renderer> factory)
        : factory(factory) {}

    void create(ID id, WindowParams params) {
        auto window = factory.create(params);
        window->show();
        windows.insert(id, std::move(window));
    }

    void destroy(ID id) {
        windows.erase(id);
    }

    ID getID(Window* window) {
        return windows.getID(window);
    }

    Window& getWindow(ID id) {
        return windows.getWindow(id);
    }

private:
    WindowFactory<Window, Renderer> factory;
    WindowMap<ID, Window> windows;
};


template <typename ID_, typename SysWindow, typename Renderer>
class Platform: public EventHandler<Window<SysWindow, Renderer>> {
public:
    using ID = ID_;
    using Window = Window<SysWindow, Renderer>;

    Platform(std::shared_ptr<Renderer> renderer)
        : windows(WindowFactory<Window, Renderer>(*this, renderer)) {}

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
        windows.create(id, params);
    }

    void destroyWindow(ID id) {
        windows.destroy(id);
    }

    void loadScene(
        ID windowID, std::unique_ptr<typename Renderer::Scene>&& scene)
    {
        windows.getWindow(windowID).loadScene(std::move(scene));
    }

    void onException(std::exception_ptr exception) noexcept override {
        outQueue.push(Event::PlatformException(exception));
    }

    void onClose(Window* sender) noexcept override {
        try {
            outQueue.push(Event::WindowClose(windows.getID(sender)));
        } catch(...) {
            outQueue.push(Event::PlatformException(std::current_exception()));
        }
    }
    
private:
    WindowManager<ID, SysWindow, Renderer> windows;
    Tools::SimpleQueue<PlatformEvent<ID>> outQueue;
};

}

// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <gui/common/platform.hpp>
#include <tools/exchange.hpp>
#include <tools/helpers.hpp>

#include <windows.h>
#include <windowsx.h>
#undef CreateWindow
#undef DestroyWindow

#include <stdexcept>
#include <memory>
#include <unordered_map>

namespace Istok::GUI::WinAPI {

template <typename Window>
class EventHandler {
public:
    virtual ~EventHandler() = default;
    virtual void onException(std::exception_ptr exception) noexcept = 0;
};


template <typename ID, typename Window>
class WindowMap {
public:
    void insert(ID id, std::unique_ptr<Window>&& window) {
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


template <typename Window>
class WindowFactory {
public:
    virtual ~WindowFactory() = default;
    virtual std::unique_ptr<Window> create(const WindowParams& params) = 0;
};


template <typename ID, typename Window>
class WindowManager {
public:
    WindowManager(std::unique_ptr<WindowFactory<Window>>&& factory)
    : factory(std::move(factory)) {}

    void create(ID id, const WindowParams& params) {
        windows.insert(id, factory->create(params));
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
    std::unique_ptr<WindowFactory<Window>> factory;
    WindowMap<ID, Window> windows;
};


template <typename Window>
class WindowFactoryBuilder {
public:
    virtual ~WindowFactoryBuilder() = default;
    virtual std::unique_ptr<WindowFactory<Window>> buildWindowFactory(
        EventHandler<Window>& eventHandler) = 0;
};


template <typename Window>
concept GUIWindow = requires {
    typename Window::Scene;
} && requires(
    Window window,
    std::unique_ptr<typename Window::Scene>&& scene,
    std::unique_ptr<WindowAreaTester>&& areaTester
) {
    {window.loadScene(std::move(scene))} -> std::same_as<void>;
    {window.setAreaTester(std::move(areaTester))} -> std::same_as<void>;
};


class WindowMessageHandler {
public:
    virtual ~WindowMessageHandler() = default;
    virtual LRESULT handleMessage(
        HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept = 0;
};


class DefaultWindowMessageHandler: public WindowMessageHandler {
public:
    LRESULT handleMessage(
        HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
    ) noexcept override {
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
};


template <typename ID>
class WindowCloseHandler: public WindowMessageHandler {
public:
    WindowCloseHandler(ID id, Tools::Queue<PlatformEvent<ID>>& outQueue)
    : id(id), outQueue(outQueue) {}

    LRESULT handleMessage(
        HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
    ) noexcept override {
        outQueue.push(PlatformEvents::WindowClose(id));
        return 0;
    }

private:
    ID id;
    Tools::Queue<PlatformEvent<ID>>& outQueue;
};


class WindowAreaTestHandler: public WindowMessageHandler {
public:
    LRESULT handleMessage(
        HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
    ) noexcept override {
        if (!tester) {
            return HTCLIENT;
        }
        RECT rect;
        GetWindowRect(hWnd, &rect);
        Position<int> position(
            GET_X_LPARAM(lParam) - rect.left,
            GET_Y_LPARAM(lParam) - rect.top);
        switch (tester->testWindowArea(position)) {
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

    void setAreaTester(std::unique_ptr<WindowAreaTester>&& tester) {
        this->tester = std::move(tester);
    }

private:
    std::unique_ptr<WindowAreaTester> tester;
};


template <typename ID>
class PlatformCommandHandler {
public:
    virtual ~PlatformCommandHandler() = default;
    virtual void handlePlatformCommand(PlatformCommand<ID> command) = 0;
};


template <typename ID, typename Window>
class WindowHandler: public PlatformCommandHandler<ID> {
public:
    WindowHandler(
        WindowManager<ID, Window>& windows,
        Tools::Queue<PlatformEvent<ID>>& outQueue
    ) : windows(windows), outQueue(outQueue) {}

    void handlePlatformCommand(PlatformCommand<ID> command) override {
        std::visit([this](const auto& x) { this->handle(x); }, command);
    }

private:
    WindowManager<ID, Window>& windows;
    Tools::Queue<PlatformEvent<ID>>& outQueue;
    
    void handle(const PlatformCommands::CreateWindow<ID>& command) {
        windows.create(command.id, command.params);
        windows.getWindow(command.id).setHandler(
            WM_CLOSE, std::make_unique<WindowCloseHandler<ID>>(
                command.id, outQueue));
    }

    void handle(const PlatformCommands::DestroyWindow<ID>& command) {
        windows.destroy(command.id);
    }

    // Fallback for commands we don't care
    template <typename T>
    void handle(const T& command) {}
};


template <typename ID_, GUIWindow Window>
class Platform: public EventHandler<Window> {
public:
    using ID = ID_;
    using Scene = Window::Scene;

    Platform(std::unique_ptr<WindowFactoryBuilder<Window>> windowFactoryBuilder)
    : windows(std::move(buildWindowFactory(std::move(windowFactoryBuilder)))) {
        handlers.push_back(std::make_unique<WindowHandler<ID, Window>>(
            windows, outQueue));
    }

    PlatformEvent<ID> getMessage() noexcept {
        while (outQueue.empty()) {
            MSG msg;
            GetMessage(&msg, NULL, 0, 0);
            if (msg.message == WM_QUIT) {
                outQueue.push(PlatformEvents::Shutdown{});
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return outQueue.take();
    }

    void sendCommand(PlatformCommand<ID> command) {
        try {
            for (auto& handler : handlers) {
                assert(handler);
                handler->handlePlatformCommand(command);
            }
        } catch(...) {
            onException(std::current_exception());
        }
    }

    void setAreaTester(
        ID id, std::unique_ptr<WindowAreaTester>&& tester) noexcept
    {
        try {
            windows.getWindow(id).setAreaTester(std::move(tester));
        } catch(...) {
            onException(std::current_exception());
        }
    }

    void loadScene(
        ID id, std::unique_ptr<Scene>&& scene) noexcept
    {
        try {
            windows.getWindow(id).loadScene(std::move(scene));
        } catch(...) {
            onException(std::current_exception());
        }
    }

    void onException(std::exception_ptr exception) noexcept override {
        outQueue.push(PlatformEvents::Exception(exception));
    }
    
private:
    std::vector<std::unique_ptr<PlatformCommandHandler<ID>>> handlers;
    WindowManager<ID, Window> windows;
    Tools::Queue<PlatformEvent<ID>> outQueue;

    std::unique_ptr<WindowFactory<Window>> buildWindowFactory(
        std::unique_ptr<WindowFactoryBuilder<Window>>&& windowFactoryBuilder
    ) {
        if (!windowFactoryBuilder) {
            throw std::runtime_error("No window factory builder found");
        }
        return windowFactoryBuilder->buildWindowFactory(*this);
    }
};

} // namespace Istok::GUI::WinAPI

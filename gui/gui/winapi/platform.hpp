// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <gui/common/platform.hpp>
#include <tools/queue.hpp>
#include <tools/helpers.hpp>

#include <windows.h>

#include <stdexcept>
#include <memory>
#include <unordered_map>

namespace Istok::GUI::WinAPI {

template <typename Window>
class EventHandler {
public:
    virtual void onException(std::exception_ptr exception) noexcept = 0;
    virtual void onClose(Window* sender) noexcept = 0;
};


template <typename Window>
concept GUIWindow = requires (
    const WindowParams& params,
    EventHandler<Window>& handler
) {
    typename Window::Renderer;
    {Window(params, handler)};
} && requires(
    Window window,
    std::unique_ptr<typename Window::Renderer>&& renderer,
    std::unique_ptr<typename Window::Renderer::Scene>&& scene,
    std::unique_ptr<WindowAreaTester>&& areaTester
) {
    {window.setRenderer(std::move(renderer))} -> std::same_as<void>;
    {window.loadScene(std::move(scene))} -> std::same_as<void>;
    {window.setAreaTester(std::move(areaTester))} -> std::same_as<void>;
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


template <typename ID, GUIWindow Window>
class WindowManager {
public:
    WindowManager(EventHandler<Window>& handler)
    : handler(handler) {}

    void create(ID id, const WindowParams& params) {
        windows.insert(id, std::make_unique<Window>(params, handler));
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
    EventHandler<Window>& handler;
    WindowMap<ID, Window> windows;
};


template <typename ID_, GUIWindow Window, typename Renderer>
class Platform: public EventHandler<Window> {
public:
    using ID = ID_;
    using Scene = Renderer::Scene;

    Platform() : windows(*this) {}

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

    void createWindow(ID id, const WindowParams& params) noexcept {
        try {
            windows.create(id, params);
            windows.getWindow(id).setRenderer(renderer.create());
        } catch(...) {
            onException(std::current_exception());
        }
    }

    void destroyWindow(ID id) noexcept {
        try {
            windows.destroy(id);
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

    void onClose(Window* sender) noexcept override {
        try {
            outQueue.push(PlatformEvents::WindowClose(windows.getID(sender)));
        } catch(...) {
            onException(std::current_exception());
        }
    }
    
private:
    Renderer renderer;
    WindowManager<ID, Window> windows;
    Tools::SimpleQueue<PlatformEvent<ID>> outQueue;
};

} // namespace Istok::GUI::WinAPI

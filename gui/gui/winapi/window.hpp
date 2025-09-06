// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "platform.hpp"

#include <memory>

namespace Istok::GUI::WinAPI {


class MessageHandler {
public:
    virtual void onClose() noexcept = 0;
    virtual void onPaint() noexcept = 0;
    virtual WindowArea onAreaTest(Position<int> position) noexcept = 0;
};


template <typename Renderer>
concept GUIRenderer = requires {
    typename Renderer::NativeHandle;
    typename Renderer::Scene;
} && requires(
    Renderer renderer,
    Renderer::NativeHandle handle,
    std::unique_ptr<typename Renderer::Scene>&& scene
) {
    {renderer.loadScene(std::move(scene))} -> std::same_as<void>;
    {renderer.prepare(handle)} -> std::same_as<void>;
    {renderer.draw(handle)} -> std::same_as<void>;
};


template <GUIRenderer Renderer>
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

    void loadScene(std::unique_ptr<typename Renderer::Scene>&& scene) {
        getRenderer().loadScene(std::move(scene));
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


template <typename SysWindow, GUIRenderer Renderer>
class WindowCore {
public:
    WindowCore(const WindowParams& params, MessageHandler& handler)
    : window(params, handler) {}

    void setRenderer(std::unique_ptr<Renderer>&& renderer) {
        if (!renderer) {
            throw std::runtime_error("No renderer provided");
        }
        renderer->prepare(window.getNativeHandle());
        data.setRenderer(std::move(renderer));
    }

    void loadScene(std::unique_ptr<typename Renderer::Scene>&& scene) {
        data.loadScene(std::move(scene));
    }
    
    void draw() {
        data.getRenderer().draw(window.getNativeHandle());
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


template <typename SysWindow, GUIRenderer Renderer_>
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

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
    typename Renderer::Scene;
} && requires(
    Renderer renderer,
    std::unique_ptr<typename Renderer::Scene>&& scene
) {
    {renderer.loadScene(std::move(scene))} -> std::same_as<void>;
    {renderer.draw()} -> std::same_as<void>;
};

template <typename SysWindow>
concept GUISysWindow = requires(
    const WindowParams& params,
    MessageHandler& handler
) {
    {SysWindow(params, handler)};
};


template <GUISysWindow SysWindow, typename Renderer>
class WindowCore {
public:
    WindowCore(
        const WindowParams& params,
        Renderer& rendererFactory,
        MessageHandler& handler
    ) : window(params, handler), renderer(rendererFactory.initWindow(window)) {}

    void loadScene(std::unique_ptr<typename Renderer::Scene>&& scene) {
        renderer->loadScene(std::move(scene));
    }
    
    void draw() {
        renderer->draw();
    }


    void setAreaTester(std::unique_ptr<WindowAreaTester>&& tester) {
        areaTester = std::move(tester);
    }

    WindowArea testArea(Position<int> position) const noexcept {
        return areaTester->testWindowArea(position);
    }

private:
    SysWindow window;
    std::unique_ptr<typename Renderer::WindowRenderer> renderer;
    std::unique_ptr<WindowAreaTester> areaTester;
};


template <GUISysWindow SysWindow, typename Renderer_>
class Window: public MessageHandler {
public:
    using Renderer = Renderer_;
    
    Window(
        const WindowParams& params,
        Renderer& renderer,
        EventHandler<Window>& handler
    ) : core(params, renderer, *this), handler(handler) {}

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

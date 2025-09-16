// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "platform.hpp"

#include <memory>

namespace Istok::GUI::WinAPI {


class MessageHandler {
public:
    virtual ~MessageHandler() = default;
    virtual void onClose() noexcept = 0;
    virtual void onPaint() noexcept = 0;
    virtual WindowArea onAreaTest(Position<int> position) noexcept = 0;
};


template <typename SysWindow>
concept GUISysWindow = requires(
    const WindowParams& params,
    MessageHandler& handler
) {
    {SysWindow(params, handler)};
};


template <typename Scene>
class Renderer {
public:
    virtual ~Renderer() = default;
    virtual void loadScene(std::unique_ptr<Scene>&& scene) = 0;
    virtual void draw() = 0;
};


template <typename SysWindow, typename Scene>
class RendererFactory {
public:
    virtual ~RendererFactory() = default;
    virtual std::unique_ptr<Renderer<Scene>> create(SysWindow& window) = 0;
};


template <GUISysWindow SysWindow, typename Scene_>
class Window: public MessageHandler {
public:
    using Scene = Scene_;
    
    Window(
        const WindowParams& params,
        RendererFactory<SysWindow, Scene>& rendererFactory,
        EventHandler<Window>& handler
    ) : core(params, *this), handler(handler),
        renderer(rendererFactory.create(core)),
        areaTester(std::make_unique<DummyAreaTester>())
    {
        if (!renderer) {
            throw std::runtime_error("Renderer not created");
        }
    }

    void onClose() noexcept override {
        handler.onClose(this);
    }

    void onPaint() noexcept override {
        try {
            renderer->draw();
        } catch(...) {
            handler.onException(std::current_exception());
        }
    }

    void loadScene(std::unique_ptr<Scene>&& scene) {
        renderer->loadScene(std::move(scene));
    }


    void setAreaTester(std::unique_ptr<WindowAreaTester>&& tester) {
        areaTester = std::move(tester);
    }

    WindowArea onAreaTest(Position<int> position) noexcept override {
        return areaTester->testWindowArea(position);
    }

private:
    SysWindow core;
    EventHandler<Window>& handler;
    std::unique_ptr<Renderer<Scene>> renderer;
    std::unique_ptr<WindowAreaTester> areaTester;

    class DummyAreaTester: public WindowAreaTester {
    public:
        WindowArea testWindowArea(
            Position<int> position
        ) const noexcept override {
            return WindowArea::client;
        }
    };
};


template <GUISysWindow SysWindow, typename Scene>
class GraphicWindowFactory: public WindowFactory<Window<SysWindow, Scene>>
{
public:
    using Window = Window<SysWindow, Scene>;

    GraphicWindowFactory(
        std::unique_ptr<RendererFactory<SysWindow, Scene>>&& rendererFactory,
        EventHandler<Window>& handler
    ) : rendererFactory(std::move(rendererFactory)), handler(handler) {
        if (!this->rendererFactory) {
            throw std::runtime_error("Renderer factory not found");
        }
    }
    
    std::unique_ptr<Window> create(const WindowParams& params) override {
        return std::make_unique<Window>(params, *rendererFactory, handler);
    }

private:
    std::unique_ptr<RendererFactory<SysWindow, Scene>> rendererFactory;
    EventHandler<Window>& handler;
};

} // namespace Istok::GUI::WinAPI

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
    virtual void loadScene(std::unique_ptr<Scene>&& scene) = 0;
    virtual void draw() = 0;
};


template <typename Scene, typename SysWindow>
class RendererFactory {
public:
    virtual std::unique_ptr<Renderer<Scene>> create(SysWindow& window) = 0;
};


template <GUISysWindow SysWindow, typename Scene>
class WindowCore {
public:
    WindowCore(
        const WindowParams& params,
        RendererFactory<Scene, SysWindow>& rendererFactory,
        MessageHandler& handler
    ) :
        window(params, handler),
        renderer(rendererFactory.create(window)),
        areaTester(std::make_unique<DummyAreaTester>())
    {}

    void loadScene(std::unique_ptr<Scene>&& scene) {
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


template <GUISysWindow SysWindow, typename Scene_>
class Window: public MessageHandler {
public:
    using Scene = Scene_;
    
    Window(
        const WindowParams& params,
        RendererFactory<Scene, SysWindow>& renderer,
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

    void loadScene(std::unique_ptr<Scene>&& scene) {
        core.loadScene(std::move(scene));
    }


    void setAreaTester(std::unique_ptr<WindowAreaTester>&& tester) {
        core.setAreaTester(std::move(tester));
    }

    WindowArea onAreaTest(Position<int> position) noexcept override {
        return core.testArea(position);
    }

private:
    WindowCore<SysWindow, Scene> core;
    EventHandler<Window>& handler;
};


template <typename SysWindow, typename RendererFactory>
class GraphicWindowFactory:
    public WindowFactory<Window<SysWindow, typename RendererFactory::Scene>>
{
public:
    using Window = Window<SysWindow, typename RendererFactory::Scene>;

    GraphicWindowFactory(EventHandler<Window>& handler)
    : handler(handler) {}
    
    std::unique_ptr<Window> create(const WindowParams& params) override {
        return std::make_unique<Window>(params, rendererFactory, handler);
    }

private:
    EventHandler<Window>& handler;
    RendererFactory rendererFactory;
};

} // namespace Istok::GUI::WinAPI

// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <tools/exchange.hpp>
#include "platform.hpp"

#include <memory>

namespace Istok::GUI::WinAPI {


class MessageHandler {
public:
    virtual ~MessageHandler() = default;
    virtual void onPaint() noexcept = 0;
    virtual WindowArea onAreaTest(Position<int> position) noexcept = 0;
};


template <typename SysWindow>
class SysWindowFactory {
public:
    virtual ~SysWindowFactory() = default;
    virtual SysWindow createSysWindow(
        const WindowParams& params, MessageHandler& handler) = 0;
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


template <typename SysWindow, typename Scene_>
class Window: public MessageHandler {
public:
    using Scene = Scene_;
    
    Window(
        const WindowParams& params,
        SysWindowFactory<SysWindow>& sysWindowFactory,
        RendererFactory<SysWindow, Scene>& rendererFactory,
        EventHandler<Window>& handler
    ) : core(sysWindowFactory.createSysWindow(params, *this)),
        handler(handler),
        renderer(rendererFactory.create(core)),
        areaTester(std::make_unique<DummyAreaTester>())
    {
        if (!renderer) {
            throw std::runtime_error("Renderer not created");
        }
    }

    void appendHandler(Tools::Handler<WindowMessage, WindowResult> handler) {
        core.appendHandler(handler);
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


template <typename SysWindow, typename Scene>
class GraphicWindowFactory: public WindowFactory<Window<SysWindow, Scene>>
{
public:
    using Window = Window<SysWindow, Scene>;

    GraphicWindowFactory(
        std::unique_ptr<SysWindowFactory<SysWindow>>&& sysWindowFactory,
        std::unique_ptr<RendererFactory<SysWindow, Scene>>&& rendererFactory,
        EventHandler<Window>& handler
    ) : rendererFactory(std::move(rendererFactory)),
        sysWindowFactory(std::move(sysWindowFactory)),
        handler(handler)
    {
        if (!this->sysWindowFactory) {
            throw std::runtime_error("System window factory not found");
        }
        if (!this->rendererFactory) {
            throw std::runtime_error("Renderer factory not found");
        }
    }
    
    std::unique_ptr<Window> create(const WindowParams& params) override {
        return std::make_unique<Window>(
            params, *sysWindowFactory, *rendererFactory, handler);
    }

private:
    std::unique_ptr<SysWindowFactory<SysWindow>> sysWindowFactory;
    std::unique_ptr<RendererFactory<SysWindow, Scene>> rendererFactory;
    EventHandler<Window>& handler;
};

} // namespace Istok::GUI::WinAPI

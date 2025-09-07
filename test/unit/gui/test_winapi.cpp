// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/winapi/window.hpp>

#include <tools/queue.hpp>
#include <string>
#include <memory>
#include <queue>
#include <format>

using namespace Istok::Tools;
using namespace Istok::GUI;
using namespace Istok::GUI::WinAPI;

namespace {

struct MockRenderer {
    using NativeHandle = std::string;
    using Scene = std::string;

    void loadScene(std::unique_ptr<Scene>&& scene) {
        this->scene = std::move(scene);
    }

    void prepare(const NativeHandle& handle) {
        log.push(std::format("prepare {}", handle));
    }
    
    void draw(const NativeHandle& handle) {
        log.push(std::format("draw {}", handle));
    }

    std::unique_ptr<Scene> scene;
    SimpleQueue<std::string> log;
};


struct MockAreaTester: public WindowAreaTester {
    WindowArea testWindowArea(Position<int> position) const noexcept override {
        if (position.x == 0 && position.y == 0) return WindowArea::hole;
        if (position.y == 0) return WindowArea::moving;
        return WindowArea::client;
    }
};


struct MockSysWindow {
    using NativeHandle = std::string;

    MockSysWindow(const WindowParams& params, MessageHandler& handler)
    : params(params) {}
    
    NativeHandle getNativeHandle() const noexcept {
        return params.title.value_or("");
    }

    WindowParams params;
};


struct MockMessageHandler: public MessageHandler {
    void onClose() noexcept override {}
    void onPaint() noexcept override {}
    WindowArea onAreaTest(Position<int> position) noexcept override {
        return WindowArea::client;
    }
};

}


static_assert(GUIRenderer<MockRenderer>);
static_assert(GUISysWindow<MockSysWindow>);


TEST_CASE("WinAPI - WindowData", "[unit][gui]") {
    WindowData<MockRenderer> data;

    REQUIRE_THROWS(data.getRenderer());
    REQUIRE_THROWS(data.loadScene(std::make_unique<MockRenderer::Scene>()));
    REQUIRE(data.testArea(Position<int>(0, 0)) == WindowArea::client);
    
    auto tmpRenderer = std::make_unique<MockRenderer>();
    auto renderer = tmpRenderer.get();
    data.setRenderer(std::move(tmpRenderer));

    REQUIRE(&data.getRenderer() == renderer);
    
    data.loadScene(std::make_unique<MockRenderer::Scene>("foo"));

    REQUIRE(*renderer->scene == "foo");

    auto tmpTester = std::make_unique<MockAreaTester>();
    data.setAreaTester(std::move(tmpTester));
 
    REQUIRE(data.testArea(Position<int>(0, 0)) == WindowArea::hole);
    REQUIRE(data.testArea(Position<int>(1, 0)) == WindowArea::moving);
    REQUIRE(data.testArea(Position<int>(1, 1)) == WindowArea::client);
}


TEST_CASE("WinAPI - WindowCore", "[unit][gui]") {
    MockMessageHandler handler;

    WindowCore<MockSysWindow, MockRenderer> core(
        WindowParams({}, "win"), handler);
    
    REQUIRE_THROWS(core.loadScene(std::make_unique<MockRenderer::Scene>()));
    REQUIRE_THROWS(core.draw());

    auto tmpRenderer = std::make_unique<MockRenderer>();
    auto renderer = tmpRenderer.get();
    core.setRenderer(std::move(tmpRenderer));

    REQUIRE(renderer->log.take() == "prepare win");
    REQUIRE(renderer->log.empty());

    core.loadScene(std::make_unique<MockRenderer::Scene>("foo"));

    REQUIRE(*renderer->scene == "foo");

    core.draw();

    REQUIRE(renderer->log.take() == "draw win");
    REQUIRE(renderer->log.empty());

    auto tmpTester = std::make_unique<MockAreaTester>();
    core.setAreaTester(std::move(tmpTester));
 
    REQUIRE(core.testArea(Position<int>(0, 0)) == WindowArea::hole);
    REQUIRE(core.testArea(Position<int>(1, 0)) == WindowArea::moving);
    REQUIRE(core.testArea(Position<int>(1, 1)) == WindowArea::client);
}

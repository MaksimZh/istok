// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/winapi/window.hpp>

#include <string>
#include <memory>

using namespace Istok::GUI;
using namespace Istok::GUI::WinAPI;

namespace {

struct MockRenderer {
    using NativeHandle = std::string;
    using Scene = std::string;

    void loadScene(std::unique_ptr<Scene>&& scene) {
        this->scene = std::move(scene);
    }

    void prepare(const NativeHandle& handle) {}
    
    void draw(const NativeHandle& handle) {}

    std::unique_ptr<Scene> scene;
};

struct MockAreaTester: public WindowAreaTester {
    WindowArea testWindowArea(Position<int> position) const noexcept override {
        if (position.x == 0 && position.y == 0) return WindowArea::hole;
        if (position.y == 0) return WindowArea::moving;
        return WindowArea::client;
    }
};

}

static_assert(GUIRenderer<MockRenderer>);

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

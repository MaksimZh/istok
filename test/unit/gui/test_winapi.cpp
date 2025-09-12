// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/winapi/window.hpp>
#include <gui/winapi/platform.hpp>

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

    void loadScene(Scene&& scene) {
        this->scene = std::move(scene);
    }

    void prepare(const NativeHandle& handle) {
        log.push(std::format("prepare {}", handle));
    }
    
    void draw(const NativeHandle& handle) {
        log.push(std::format("draw {}", handle));
    }

    Scene scene;
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


std::string str(void* x) {
    return std::format("onClose {:#x}", reinterpret_cast<std::uintptr_t>(x));
}


template <typename Window>
struct MockEventHandler: public EventHandler<Window> {
    void onException(std::exception_ptr exception) noexcept override {
        log.push("onException");
    }

    void onClose(Window* sender) noexcept override {
        log.push(std::format("onClose {}", str(sender)));
    }

    SimpleQueue<std::string> log;
};

}


static_assert(GUIRenderer<MockRenderer>);
static_assert(GUISysWindow<MockSysWindow>);
static_assert(GUIWindow<Window<MockSysWindow, MockRenderer>>);


TEST_CASE("WinAPI - WindowData", "[unit][gui]") {
    WindowData<MockRenderer> data;

    SECTION("Rendering") {
        REQUIRE_THROWS(data.getRenderer());
        REQUIRE_THROWS(data.loadScene(MockRenderer::Scene()));
        REQUIRE(data.testArea(Position<int>(0, 0)) == WindowArea::client);
        
        auto tmpRenderer = std::make_unique<MockRenderer>();
        auto renderer = tmpRenderer.get();
        data.setRenderer(std::move(tmpRenderer));

        REQUIRE(&data.getRenderer() == renderer);
        
        data.loadScene(MockRenderer::Scene("foo"));

        REQUIRE(renderer->scene == "foo");
    }

    SECTION("Area test") {
        auto tmpTester = std::make_unique<MockAreaTester>();
        data.setAreaTester(std::move(tmpTester));
 
        REQUIRE(data.testArea(Position<int>(0, 0)) == WindowArea::hole);
        REQUIRE(data.testArea(Position<int>(1, 0)) == WindowArea::moving);
        REQUIRE(data.testArea(Position<int>(1, 1)) == WindowArea::client);
    }
}


TEST_CASE("WinAPI - WindowCore", "[unit][gui]") {
    MockMessageHandler handler;
    WindowCore<MockSysWindow, MockRenderer> core(
        WindowParams({}, "win"), handler);

    SECTION("Rendering") {
        REQUIRE_THROWS(core.loadScene(MockRenderer::Scene()));
        REQUIRE_THROWS(core.draw());

        auto tmpRenderer = std::make_unique<MockRenderer>();
        auto renderer = tmpRenderer.get();

        REQUIRE(renderer->log.empty());
        core.setRenderer(std::move(tmpRenderer));
        REQUIRE(renderer->log.take() == "prepare win");
        REQUIRE(renderer->log.empty());

        core.loadScene(MockRenderer::Scene("foo"));
        REQUIRE(renderer->scene == "foo");

        REQUIRE(renderer->log.empty());
        core.draw();
        REQUIRE(renderer->log.take() == "draw win");
        REQUIRE(renderer->log.empty());
    }

    SECTION("Area test") {
        REQUIRE(core.testArea(Position<int>(0, 0)) == WindowArea::client);

        auto tmpTester = std::make_unique<MockAreaTester>();
        core.setAreaTester(std::move(tmpTester));
    
        REQUIRE(core.testArea(Position<int>(0, 0)) == WindowArea::hole);
        REQUIRE(core.testArea(Position<int>(1, 0)) == WindowArea::moving);
        REQUIRE(core.testArea(Position<int>(1, 1)) == WindowArea::client);
    }
}


TEST_CASE("WinAPI - Window", "[unit][gui]") {
    using Win = Window<MockSysWindow, MockRenderer>;
    MockEventHandler<Win> handler;
    Win window(WindowParams({}, "win"), handler);

    SECTION("Close") {
        REQUIRE(handler.log.empty());
        window.onClose();
        REQUIRE(handler.log.take() == std::format("onClose {}", str(&window)));
        REQUIRE(handler.log.empty());
    }

    SECTION("Rendering") {
        REQUIRE_THROWS(window.loadScene(MockRenderer::Scene()));
    
        REQUIRE(handler.log.empty());
        window.onPaint();
        REQUIRE(handler.log.take() == "onException");
        REQUIRE(handler.log.empty());

        auto tmpRenderer = std::make_unique<MockRenderer>();
        auto renderer = tmpRenderer.get();

        REQUIRE(renderer->log.empty());
        window.setRenderer(std::move(tmpRenderer));
        REQUIRE(renderer->log.take() == "prepare win");
        REQUIRE(renderer->log.empty());

        window.loadScene(MockRenderer::Scene("foo"));
        REQUIRE(renderer->scene == "foo");

        REQUIRE(renderer->log.empty());
        window.onPaint();
        REQUIRE(renderer->log.take() == "draw win");
        REQUIRE(renderer->log.empty());
    }

    SECTION("Area test") {
        REQUIRE(window.onAreaTest(Position<int>(0, 0)) == WindowArea::client);

        auto tmpTester = std::make_unique<MockAreaTester>();
        window.setAreaTester(std::move(tmpTester));
    
        REQUIRE(window.onAreaTest(Position<int>(0, 0)) == WindowArea::hole);
        REQUIRE(window.onAreaTest(Position<int>(1, 0)) == WindowArea::moving);
        REQUIRE(window.onAreaTest(Position<int>(1, 1)) == WindowArea::client);
    }
}


TEST_CASE("WinAPI - WindowMap", "[unit][gui]") {
    WindowMap<int, int> map;

    SECTION("Empty") {
        auto a = new int(1);
        REQUIRE_THROWS(map.getID(a));
        REQUIRE_THROWS(map.getWindow(1));
        REQUIRE_THROWS(map.erase(1));
    }

    SECTION("Single") {
        auto pa = std::make_unique<int>(1);
        auto pb = std::make_unique<int>(2);
        auto a = pa.get();
        auto b = pb.get();

        map.insert(1, std::move(pa));

        REQUIRE(map.getID(a) == 1);
        REQUIRE(&map.getWindow(1) == a);
        REQUIRE_THROWS(map.insert(1, std::move(pb)));
        REQUIRE_THROWS(map.getID(b));
        REQUIRE_THROWS(map.getWindow(2));
        REQUIRE_THROWS(map.erase(2));

        map.erase(1);

        REQUIRE_THROWS(map.getID(a));
        REQUIRE_THROWS(map.getWindow(1));
        REQUIRE_THROWS(map.erase(1));
    }

    SECTION("Many") {
        auto pa = std::make_unique<int>(1);
        auto pb = std::make_unique<int>(2);
        auto pc = std::make_unique<int>(3);
        auto a = pa.get();
        auto b = pb.get();
        auto c = pc.get();

        map.insert(1, std::move(pa));
        map.insert(2, std::move(pb));
        map.insert(3, std::move(pc));

        REQUIRE(map.getID(a) == 1);
        REQUIRE(map.getID(b) == 2);
        REQUIRE(map.getID(c) == 3);
        REQUIRE(&map.getWindow(1) == a);
        REQUIRE(&map.getWindow(2) == b);
        REQUIRE(&map.getWindow(3) == c);

        map.erase(2);
        REQUIRE(map.getID(a) == 1);
        REQUIRE_THROWS(map.getID(b));
        REQUIRE(map.getID(c) == 3);
        REQUIRE(&map.getWindow(1) == a);
        REQUIRE_THROWS(map.getWindow(2));
        REQUIRE(&map.getWindow(3) == c);
    }
}


TEST_CASE("WinAPI - WindowManager", "[unit][gui]") {
    using Win = Window<MockSysWindow, MockRenderer>;
    MockEventHandler<Win> handler;
    WindowManager<int, Win> manager(handler);

    SECTION("Empty") {
        Win foo(WindowParams({}, "foo"), handler);
        REQUIRE_THROWS(manager.getID(&foo));
        REQUIRE_THROWS(manager.getWindow(42));
        REQUIRE_THROWS(manager.destroy(42));
    }

    SECTION("Single window") {
        Win foo(WindowParams({}, "foo"), handler);
        manager.create(1, WindowParams({}, "win"));

        REQUIRE_THROWS(manager.getID(&foo));
        REQUIRE_THROWS(manager.getWindow(42));
        REQUIRE_THROWS(manager.destroy(42));

        Win* window = &manager.getWindow(1);
        REQUIRE(manager.getID(window) == 1);

        manager.destroy(1);
        REQUIRE_THROWS(manager.getID(window));
        REQUIRE_THROWS(manager.getWindow(1));
        REQUIRE_THROWS(manager.destroy(1));
    }

    SECTION("Multiply window") {
        Win foo(WindowParams({}, "foo"), handler);
        manager.create(1, WindowParams({}, "a"));
        manager.create(2, WindowParams({}, "b"));
        manager.create(3, WindowParams({}, "c"));

        REQUIRE_THROWS(manager.getID(&foo));
        REQUIRE_THROWS(manager.getWindow(42));
        REQUIRE_THROWS(manager.destroy(42));

        REQUIRE(manager.getID(&manager.getWindow(1)) == 1);
        REQUIRE(manager.getID(&manager.getWindow(2)) == 2);
        REQUIRE(manager.getID(&manager.getWindow(3)) == 3);

        Win* window = &manager.getWindow(2);
        manager.destroy(2);
        REQUIRE(manager.getID(&manager.getWindow(1)) == 1);
        REQUIRE(manager.getID(&manager.getWindow(3)) == 3);
        REQUIRE_THROWS(manager.getID(window));
        REQUIRE_THROWS(manager.getWindow(2));
        REQUIRE_THROWS(manager.destroy(2));
    }
}


TEST_CASE("WinAPI - Platform", "[unit][gui]") {
    using Win = Window<MockSysWindow, MockRenderer>;
    MockEventHandler<Win> handler;
    Platform<int, Win> platform;
}

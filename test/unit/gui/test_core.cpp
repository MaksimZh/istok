// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/common/core.hpp>
#include <tools/queue.hpp>
#include <tools/helpers.hpp>

using namespace Istok::Tools;
using namespace Istok::GUI;

#include <mutex>
#include <string>
#include <format>
#include <variant>


namespace {

using DebugQueue = SyncWaitingQueue<std::string>;

template <typename WindowID_>
class MockPlatform {
public:
    using WindowID = WindowID_;
    using InQueue = SyncWaitingQueue<GUIMessage<WindowID>>;

    std::shared_ptr<DebugQueue> debugQueue;

    MockPlatform(GUIHandler<WindowID>& handler)
        : instanceGetter(this),
        handler(&handler),
        queue(std::make_shared<InQueue>()),
        debugQueue(std::make_shared<DebugQueue>())
    {
        debugQueue->push("create");
    }

    ~MockPlatform() {
        debugQueue->push("destroy");
    }

    static MockPlatform* release() {
        return InstanceGetter<MockPlatform>::release();
    }

    MockPlatform(const MockPlatform&) = delete;
    MockPlatform& operator=(const MockPlatform&) = delete;
    MockPlatform(MockPlatform&&) = delete;
    MockPlatform& operator=(MockPlatform&&) = delete;
    
    std::shared_ptr<InQueue> getQueue() {
        return queue;
    }

    void run() {
        debugQueue->push("run");
        running = true;
        if (sync) {
            return;
        }
        while (running) {
            this->handler->onMessage(queue->take());
        }
    }

    /**
     * @brief Enter synchronous mode
     * 
     * Must be called before `run`.
     * 
     */
    void syncStart() {
        sync = true;
    }

    void stop() noexcept {
        debugQueue->push("stop");
        running = false;
    }

    void newWindow(WindowID id, WindowParams params) {
        if (id < 0) {
            throw std::runtime_error("newWindow failed");
        }
        debugQueue->push(std::format("new window {}", id));
    }

    void destroyWindow(WindowID id) {
        if (id < 0) {
            throw std::runtime_error("destroyWindow failed");
        }
        debugQueue->push(std::format("destroy window {}", id));
    }

private:
    InstanceGetter<MockPlatform> instanceGetter;

    GUIHandler<WindowID>* handler;
    std::shared_ptr<InQueue> queue;
    bool running = false;
    bool sync = false;
};

}


TEST_CASE("GUI - Core", "[unit][gui]") {
    using Platform = MockPlatform<int>;
    auto appQueue = std::make_shared<AppQueue<int>>();
    GUICore<Platform> core(appQueue);
    auto platform = Platform::release();
    auto queue = core.getQueue();
    auto debugQueue = platform->debugQueue;
    REQUIRE(debugQueue->take() == "create");
    
    platform->syncStart();
    core.run();
    REQUIRE(debugQueue->take() == "run");

    SECTION("exit") {
        core.onMessage(Message::GUIExit{});
        REQUIRE(debugQueue->take() == "stop");
    }

    SECTION("new window") {
        core.onMessage(Message::GUINewWindow<int>(42, WindowParams{}));
        REQUIRE(debugQueue->take() == "new window 42");
    }

    SECTION("new window fail") {
        core.onMessage(Message::GUINewWindow<int>(-1, WindowParams{}));
        auto msg = appQueue->take();
        REQUIRE(std::holds_alternative<Message::AppGUIException>(msg));
        REQUIRE_THROWS_MATCHES(
            std::rethrow_exception(
                std::get<Message::AppGUIException>(msg).exception),
            std::runtime_error,
            Catch::Matchers::Predicate<std::runtime_error>(
                [](const std::runtime_error& e) {
                    return std::string(e.what()) == "newWindow failed";
                },
                "received a message about newWindow fail"
            )
        );
    }

    SECTION("destroy window") {
        core.onMessage(Message::GUIDestroyWindow<int>(42));
        REQUIRE(debugQueue->take() == "destroy window 42");
    }

    SECTION("destroy window fail") {
        core.onMessage(Message::GUIDestroyWindow<int>(-1));
        auto msg = appQueue->take();
        REQUIRE(std::holds_alternative<Message::AppGUIException>(msg));
        REQUIRE_THROWS_MATCHES(
            std::rethrow_exception(
                std::get<Message::AppGUIException>(msg).exception),
            std::runtime_error,
            Catch::Matchers::Predicate<std::runtime_error>(
                [](const std::runtime_error& e) {
                    return std::string(e.what()) == "destroyWindow failed";
                },
                "received a message about newWindow fail"
            )
        );
    }

    SECTION("on close") {
        core.onWindowClose(42);
        auto msg = appQueue->take();
        REQUIRE(std::holds_alternative<Message::AppWindowClosed<int>>(msg));
        REQUIRE(std::get<Message::AppWindowClosed<int>>(msg).id == 42);
    }
}


TEST_CASE("GUI - GUI", "[unit][gui]") {
    using Platform = MockPlatform<int>;
    std::shared_ptr<DebugQueue> debugQueue;
    {
        GUIFor<Platform> gui;
        Platform* platform = Platform::release();
        debugQueue = platform->debugQueue;
        REQUIRE(debugQueue->take() == "create");
        REQUIRE(debugQueue->take() == "run");
        gui.newWindow(42, WindowParams{});
        REQUIRE(debugQueue->take() == "new window 42");
        gui.destroyWindow(42);
        REQUIRE(debugQueue->take() == "destroy window 42");
    }
    REQUIRE(debugQueue->take() == "stop");
    REQUIRE(debugQueue->take() == "destroy");
}

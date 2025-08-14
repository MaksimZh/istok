// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/core/launcher.hpp>

using namespace Istok::Tools;
using namespace Istok::GUI;

#include <mutex>
#include <string>
#include <format>


namespace {

using AppQueue = SyncWaitingQueue<AppMessage<int>>;

class MockPlatform {
public:
    using InQueue = SyncWaitingQueue<GUIMessage<int>>;
    using DebugQueue = SyncWaitingQueue<std::string>;
    std::shared_ptr<DebugQueue> debugQueue;

    MockPlatform()
        : queue(std::make_shared<InQueue>()),
        debugQueue(std::make_shared<DebugQueue>())
    {
        std::unique_lock lock(mut);
        debugQueue->push("create");
        cv.wait(lock, [] { return instance == nullptr; });
        instance = this;
    }

    ~MockPlatform() {
        std::lock_guard lock(mut);
        debugQueue->push("destroy");
        if (instance == this) {
            instance = nullptr;
            cv.notify_all();
        }
    }

    static MockPlatform* release() {
        std::lock_guard lock(mut);
        MockPlatform* tmp = instance;
        instance = nullptr;
        cv.notify_all();
        return tmp;
    }

    MockPlatform(const MockPlatform&) = delete;
    MockPlatform& operator=(const MockPlatform&) = delete;
    MockPlatform(MockPlatform&&) = delete;
    MockPlatform& operator=(MockPlatform&&) = delete;
    
    std::shared_ptr<InQueue> getInQueue() {
        return queue;
    }

    void runStart(WindowMessageHandler<int>& handler) {
        debugQueue->push("run");
        this->handler = &handler;
        running = true;
    }

    void run(WindowMessageHandler<int>& handler) {
        runStart(handler);
        while (running) {
            this->handler->onMessage(queue->take());
        }
    }

    void sendQueue(GUIMessage<int> msg) {
        handler->onMessage(msg);
    }

    void sendClose(int id) {
        handler->onClose(id);
    }

    void stop() {
        debugQueue->push("stop");
        running = false;
    }

    void newWindow(int id, WindowParams params) {
        debugQueue->push(std::format("new window {}", id));
    }

    void destroyWindow(int id) {
        debugQueue->push(std::format("destroy window {}", id));
    }

private:
    static MockPlatform* instance;
    static std::mutex mut;
    static std::condition_variable cv;

    WindowMessageHandler<int>* handler;
    std::shared_ptr<InQueue> queue;
    bool running;
};

MockPlatform* MockPlatform::instance = nullptr;
std::mutex MockPlatform::mut;
std::condition_variable MockPlatform::cv;

}


TEST_CASE("GUI - Handler", "[unit][gui]") {
    MockPlatform platform;
    std::shared_ptr<MockPlatform::DebugQueue> debugQueue =
        MockPlatform::release()->debugQueue;
    REQUIRE(debugQueue->take() == "create");
    std::shared_ptr<AppQueue> appQueue = std::make_shared<AppQueue>();
    Handler<int, MockPlatform, AppQueue> handler(platform, appQueue);
    platform.runStart(handler);
    REQUIRE(debugQueue->take() == "run");

    SECTION("exit") {
        platform.sendQueue(Message::GUIExit{});
        REQUIRE(debugQueue->take() == "stop");
    }

    SECTION("new window") {
        platform.sendQueue(Message::GUINewWindow<int>(42, WindowParams{}));
        REQUIRE(debugQueue->take() == "new window 42");
    }

    SECTION("destroy window") {
        platform.sendQueue(Message::GUIDestroyWindow<int>(42));
        REQUIRE(debugQueue->take() == "destroy window 42");
    }

    SECTION("on close") {
        platform.sendQueue(Message::GUINewWindow<int>(42, WindowParams{}));
        REQUIRE(debugQueue->take() == "new window 42");
        platform.sendClose(42);
        auto msg = appQueue->take();
        REQUIRE(std::holds_alternative<Message::AppWindowClosed<int>>(msg));
        REQUIRE(std::get<Message::AppWindowClosed<int>>(msg).id == 42);
    }
}


TEST_CASE("GUI - GUI", "[unit][gui]") {
    std::shared_ptr<MockPlatform::DebugQueue> debugQueue;
    {
        GUIFor<int, MockPlatform> gui;
        MockPlatform* platform = MockPlatform::release();
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

// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/core/launcher.hpp>

using namespace Istok::Tools;
using namespace Istok::GUI;

#include <string>
#include <format>


namespace {

using AppQueue = SyncWaitingQueue<AppMessage<int>>;

class MockPlatform {
public:
    static SyncWaitingQueue<std::string> debugQueue;

    using InQueue = SyncWaitingQueue<GUIMessage<int>>;

    MockPlatform() : queue(std::make_shared<InQueue>()) {
        debugQueue.push("create");
    }

    ~MockPlatform() {
        debugQueue.push("destroy");
    }
    
    std::shared_ptr<InQueue> getInQueue() {
        return queue;
    }

    void runStart(WindowMessageHandler<int>& handler) {
        debugQueue.push("run");
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
        debugQueue.push("stop");
        running = false;
    }

    void newWindow(int id, WindowParams params) {
        debugQueue.push(std::format("new window {}", id));
    }

    void destroyWindow(int id) {
        debugQueue.push(std::format("destroy window {}", id));
    }

private:
    WindowMessageHandler<int>* handler;
    std::shared_ptr<InQueue> queue;
    bool running;
};

SyncWaitingQueue<std::string> MockPlatform::debugQueue;

}


TEST_CASE("GUI - Handler", "[unit][gui]") {
    MockPlatform platform;
    platform.debugQueue.clean();
    std::shared_ptr<AppQueue> appQueue = std::make_shared<AppQueue>();
    Handler<int, MockPlatform, AppQueue> handler(platform, appQueue);
    platform.runStart(handler);
    REQUIRE(platform.debugQueue.take() == "run");

    SECTION("exit") {
        platform.sendQueue(Message::GUIExit{});
        REQUIRE(platform.debugQueue.take() == "stop");
    }

    SECTION("new window") {
        platform.sendQueue(Message::GUINewWindow<int>(42, WindowParams{}));
        REQUIRE(platform.debugQueue.take() == "new window 42");
    }

    SECTION("destroy window") {
        platform.sendQueue(Message::GUIDestroyWindow<int>(42));
        REQUIRE(platform.debugQueue.take() == "destroy window 42");
    }

    SECTION("on close") {
        platform.sendQueue(Message::GUINewWindow<int>(42, WindowParams{}));
        REQUIRE(platform.debugQueue.take() == "new window 42");
        platform.sendClose(42);
        auto msg = appQueue->take();
        REQUIRE(std::holds_alternative<Message::AppWindowClosed<int>>(msg));
        REQUIRE(std::get<Message::AppWindowClosed<int>>(msg).id == 42);
    }
}


TEST_CASE("GUI - GUI", "[unit][gui]") {
    MockPlatform::debugQueue.clean();
    {
        GUIFor<int, MockPlatform> gui;
        REQUIRE(MockPlatform::debugQueue.take() == "create");
        REQUIRE(MockPlatform::debugQueue.take() == "run");
        gui.newWindow(42, WindowParams{});
        REQUIRE(MockPlatform::debugQueue.take() == "new window 42");
        gui.destroyWindow(42);
        REQUIRE(MockPlatform::debugQueue.take() == "destroy window 42");
    }
    REQUIRE(MockPlatform::debugQueue.take() == "stop");
    REQUIRE(MockPlatform::debugQueue.take() == "destroy");
}

// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/core/launcher.hpp>

using namespace Istok::Tools;
using namespace Istok::GUI;

#include <string>
#include <format>


namespace {

class MockPlatform {
public:
    SimpleQueue<std::string> debugQueue;

    void run(WindowMessageHandler<int>& handler) {
        this->handler = &handler;
    }

    void sendQueue(GUIMessage<int> msg) {
        handler->handleMessage(msg);
    }

    void stop() {
        debugQueue.push("stop");
    }

    void newWindow(int id, WindowParams params) {
        debugQueue.push(std::format("new window {}", id));
    }

    void destroyWindow(int id) {
        debugQueue.push(std::format("destroy window {}", id));
    }

private:
    WindowMessageHandler<int>* handler = nullptr;
};

}


TEST_CASE("GUI - Handler", "[unit][gui]") {
    MockPlatform platform;
    std::shared_ptr<AppQueue> appQueue = std::make_shared<AppQueue>();
    Handler<int, MockPlatform, AppQueue> handler(platform, appQueue);
    platform.run(handler);

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
}


namespace {

class FakePlatform {
public:
    static SyncWaitingQueue<std::string> debugQueue;
    
    static void cleanDebugQueue() {
        while (!debugQueue.empty()) {
            debugQueue.take();
        }
    }

    using InQueue = SyncWaitingQueue<GUIMessage<int>>;

    FakePlatform() : queue(std::make_shared<InQueue>()) {
        debugQueue.push("create");
    }

    ~FakePlatform() {
        debugQueue.push("destroy");
    }
    
    std::shared_ptr<InQueue> getInQueue() {
        return queue;
    }

    void run(WindowMessageHandler<int>& handler) {
        debugQueue.push("run");
        running = true;
        while (running) {
            handler.handleMessage(queue->take());
        }
    }

    void stop() {
        debugQueue.push("stop");
        running = false;
    }

    void newWindow(int id, WindowParams params) {}
    void destroyWindow(int id) {}

private:
    std::shared_ptr<InQueue> queue;
    bool running;
};

SyncWaitingQueue<std::string> FakePlatform::debugQueue;

}


TEST_CASE("GUI - GUI", "[unit][gui]") {
    FakePlatform::cleanDebugQueue();
    {
        GUIFor<int, FakePlatform> gui;
        REQUIRE(FakePlatform::debugQueue.take() == "create");
        REQUIRE(FakePlatform::debugQueue.take() == "run");
    }
    REQUIRE(FakePlatform::debugQueue.take() == "stop");
    REQUIRE(FakePlatform::debugQueue.take() == "destroy");
}

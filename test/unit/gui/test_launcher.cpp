// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/core/launcher.hpp>

using namespace Istok::Tools;
using namespace Istok::GUI;

#include <string>
#include <semaphore>


namespace {

class MockPlatform {
public:
    SimpleQueue<std::string> debugQueue;

    void run(WindowMessageHandler& handler) {
        this->handler = &handler;
    }

    void sendQueue(GUIMessage msg) {
        handler->handleMessage(msg);
    }

    void stop() {
        debugQueue.push("stop");
    }

private:
    WindowMessageHandler* handler = nullptr;
};

}


TEST_CASE("GUI - Handler", "[unit][gui]") {
    MockPlatform platform;
    std::shared_ptr<AppQueue> appQueue = std::make_shared<AppQueue>();
    Handler handler(platform, appQueue);
    platform.run(handler);

    SECTION("exit") {
        platform.sendQueue(Message::GUIExit{});
        REQUIRE(platform.debugQueue.take() == "stop");
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

    using InQueue = SyncWaitingQueue<GUIMessage>;

    FakePlatform() : queue(std::make_shared<InQueue>()) {
        debugQueue.push("create");
    }

    ~FakePlatform() {
        debugQueue.push("destroy");
    }
    
    std::shared_ptr<InQueue> getInQueue() {
        return queue;
    }

    void run(WindowMessageHandler& handler) {
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

private:
    std::shared_ptr<InQueue> queue;
    bool running;
};

SyncWaitingQueue<std::string> FakePlatform::debugQueue;

}


TEST_CASE("GUI - GUI", "[unit][gui]") {
    FakePlatform::cleanDebugQueue();
    {
        GUIFor<FakePlatform> gui;
        REQUIRE(FakePlatform::debugQueue.take() == "create");
        REQUIRE(FakePlatform::debugQueue.take() == "run");
    }
    REQUIRE(FakePlatform::debugQueue.take() == "stop");
    REQUIRE(FakePlatform::debugQueue.take() == "destroy");
}

// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/core/launcher.hpp>

using namespace Istok::Tools;
using namespace Istok::GUI;

#include <string>
#include <semaphore>


namespace {
/*
class MockPlatform {
public:
    static SyncWaitingQueue<std::string> debugQueue;
    
    static void cleanDebugQueue() {
        while (!debugQueue.empty()) {
            debugQueue.take();
        }
    }

    class Notifier {
    public:
        Notifier(MockPlatform& platform) : platform(&platform) {}
        
        void operator()() {}
    
    private:
        MockPlatform* platform;
    };

    using InQueue = SyncNotifyingQueue<GUIMessage, Notifier>;

    MockPlatform() : queue(std::make_shared<InQueue>(Notifier(*this))) {
        debugQueue.push("create");
    }

    ~MockPlatform() {
        debugQueue.push("destroy");
    }
    
    std::shared_ptr<InQueue> getInQueue() {
        return queue;
    }

    void run(WindowMessageHandler& handler) {
        debugQueue.push("run");
        this->handler = &handler;
    }

    void sendQueue() {
        assert(handler);
        assert(!queue->empty());
        handler->handleMessage(queue->take());
    }

    void stop() {
        debugQueue.push("stop");
    }

private:
    std::shared_ptr<InQueue> queue;
    WindowMessageHandler* handler = nullptr;
};

SyncWaitingQueue<std::string> MockPlatform::debugQueue;
*/
}

/*
TEST_CASE("GUI - Handler", "[unit][gui]") {
    MockPlatform::cleanDebugQueue();
    MockPlatform platform;
    std::shared_ptr<AppQueue> appQueue = std::make_shared<AppQueue>();
    auto guiQueue = platform.getInQueue();
    Handler handler(platform, appQueue);
    platform.run(handler);
    REQUIRE(MockPlatform::debugQueue.take() == "create");
    REQUIRE(MockPlatform::debugQueue.take() == "run");

    SECTION("exit") {
        guiQueue->push(Message::GUIExit{});
        platform.sendQueue();
        REQUIRE(MockPlatform::debugQueue.take() == "stop");
    }
}
*/



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

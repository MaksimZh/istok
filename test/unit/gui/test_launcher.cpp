// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/core/launcher.hpp>

using namespace Istok::Tools;
using namespace Istok::GUI;

#include <string>


namespace {

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
        debugQueue.push("create platform");
    }

    ~MockPlatform() {
        debugQueue.push("destroy platform");
    }
    
    std::shared_ptr<InQueue> getInQueue() {
        return queue;
    }

    void run(WindowMessageHandler& handler) {
        debugQueue.push("run platform");
    }

private:
    std::shared_ptr<InQueue> queue;
};

SyncWaitingQueue<std::string> MockPlatform::debugQueue;

}


TEST_CASE("GUI - Handler", "[unit][gui]") {
    MockPlatform platform;
    std::shared_ptr<AppQueue> appQueue = std::make_shared<AppQueue>();
    Handler core(platform, appQueue);
}


TEST_CASE("GUI - GUI", "[unit][gui]") {
    MockPlatform::cleanDebugQueue();
    {
        GUIFor<MockPlatform> gui;
        REQUIRE(MockPlatform::debugQueue.take() == "create platform");
        REQUIRE(MockPlatform::debugQueue.take() == "run platform");
    }
    REQUIRE(MockPlatform::debugQueue.take() == "destroy platform");
}

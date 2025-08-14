// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/core/launcher.hpp>

using namespace Istok::Tools;
using namespace Istok::GUI;

#include <string>


namespace {

class MockPlatform {
public:
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
    
    void setMessageHandler(WindowMessageHandler& handler) {}

    std::shared_ptr<InQueue> getInQueue() {
        return queue;
    }

    static SyncWaitingQueue<std::string> debugQueue;
    
    static void cleanDebugQueue() {
        while (!debugQueue.empty()) {
            debugQueue.take();
        }
    }

private:
    std::shared_ptr<InQueue> queue;
};

SyncWaitingQueue<std::string> MockPlatform::debugQueue;

}


TEST_CASE("GUI - Core", "[unit][gui]") {
    MockPlatform platform;
    std::shared_ptr<AppQueue> appQueue = std::make_shared<AppQueue>();
    Core<MockPlatform, AppQueue> core(platform, appQueue);
}


TEST_CASE("GUI - GUI", "[unit][gui]") {
    MockPlatform::cleanDebugQueue();
    {
        GUIFor<MockPlatform> gui;
        REQUIRE(MockPlatform::debugQueue.take() == "create platform");
    }
    REQUIRE(MockPlatform::debugQueue.take() == "destroy platform");
}

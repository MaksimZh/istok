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

    MockPlatform() : queue(std::make_shared<InQueue>(Notifier(*this))) {}
    
    void setMessageHandler(WindowMessageHandler& handler) {}

    std::shared_ptr<InQueue> getInQueue() {
        return queue;
    }

private:
    std::shared_ptr<InQueue> queue;
};

}


TEST_CASE("GUI - Core", "[unit][gui]") {
    MockPlatform platform;
    std::shared_ptr<AppQueue> appQueue = std::make_shared<AppQueue>();
    Core<MockPlatform, AppQueue> core(platform, appQueue);
}


TEST_CASE("GUI - GUI", "[unit][gui]") {
    std::unique_ptr<MockPlatform> platform = std::make_unique<MockPlatform>();
    PlatformGUI gui(std::move(platform));
}

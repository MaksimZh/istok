// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/core/launcher.hpp>

using namespace Istok::Tools;
using namespace Istok::GUI;

#include <string>


namespace {

class FakePlatform {
public:
    class Notifier {
    public:
        Notifier(FakePlatform& platform) : platform(&platform) {}

    private:
        FakePlatform* platform;
    };

    using InQueue = SyncNotifyingQueue<std::string, Notifier>;

    FakePlatform() : queue(std::make_shared<InQueue>(Notifier(*this))) {}
    
    void setMessageHandler(WindowMessageHandler& handler) {}

    std::shared_ptr<InQueue> getInQueue() {
        return queue;
    }

private:
    std::shared_ptr<InQueue> queue;
};

}


TEST_CASE("GUI - Core", "[unit][gui]") {
    FakePlatform platform;
    std::shared_ptr<AppQueue> appQueue = std::make_shared<AppQueue>();
    Core<FakePlatform, AppQueue> core(platform, appQueue);
}


TEST_CASE("GUI - GUI", "[unit][gui]") {
    GUIFor<FakePlatform> gui;
}

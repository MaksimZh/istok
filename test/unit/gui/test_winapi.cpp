// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/winapi/platform.hpp>
#include <gui/common/message.hpp>
#include <gui/common/core.hpp>

using namespace Istok::GUI;
using namespace Istok::GUI::WinAPI;

#include <memory>


namespace {

class MockNotifierWindow {
public:
    MockNotifierWindow(int& counter) : counter(counter) {}
    
    void postQueueNotification() {
        ++counter;
    }
private:
    int& counter;
};

}

TEST_CASE("WinAPI - Notifier", "[unit][gui]") {
    int counter = 0;
    auto window = std::make_shared<MockNotifierWindow>(counter);
    Notifier notifier(window);
    REQUIRE(counter == 0);
    notifier();
    REQUIRE(counter == 1);
    notifier();
    REQUIRE(counter == 2);
    window.reset();
    notifier();
    REQUIRE(counter == 2);
}

/*
TEST_CASE("WinAPI - Platform", "[unit][gui]") {
    using P = Platform<int>;
    auto appQueue = std::make_shared<AppQueue<int>>();
    GUICore<P> core(appQueue);
}
*/

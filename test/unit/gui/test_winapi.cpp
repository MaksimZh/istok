// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/winapi/platform.hpp>
#include <gui/common/message.hpp>
#include <gui/common/core.hpp>

using namespace Istok::GUI;
using namespace Istok::GUI::WinAPI;

#include <memory>


TEST_CASE("WinAPI - Notifier", "[unit][gui]") {
    struct Window {
        int& counter;
        void postQueueNotification() { ++counter; }
    };

    int counter = 0;
    auto window = std::make_shared<Window>(counter);
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
TEST_CASE("WinAPI - Queue proxy", "[unit][gui]") {
    QueueProxy<int, NotifierWindow> proxy;
}
*/

/*
TEST_CASE("WinAPI - Platform", "[unit][gui]") {
    using P = Platform<int>;
    auto appQueue = std::make_shared<AppQueue<int>>();
    GUICore<P> core(appQueue);
}
*/

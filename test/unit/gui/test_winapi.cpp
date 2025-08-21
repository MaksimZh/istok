// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/winapi/platform.hpp>
#include <gui/common/message.hpp>
#include <gui/common/core.hpp>

using namespace Istok::GUI;
using namespace Istok::GUI::WinAPI;

#include <memory>
#include <string>

namespace {

template <typename WindowID>
class MockHandler: public GUIHandler<WindowID> {
public:
    SyncWaitingQueue<std::string> debugQueue;
    
    void onMessage(GUIMessage<WindowID> msg) noexcept override {
        if (std::holds_alternative<Message::GUIExit>(msg)) {
            debugQueue.push("exit");
            return;
        }
        if (std::holds_alternative<Message::GUINewWindow<WindowID>>(msg)) {
            debugQueue.push(std::format(
                "new window {}",
                std::get<Message::GUINewWindow<WindowID>>(msg).id));
            return;
        }
        if (std::holds_alternative<Message::GUIDestroyWindow<WindowID>>(msg)) {
            debugQueue.push(std::format(
                "destroy window {}",
                std::get<Message::GUIDestroyWindow<WindowID>>(msg).id));
            return;
        }
    };
    
    void onWindowClose(WindowID id) noexcept override {
        debugQueue.push(std::format("window close {}", id));
    };
};

}


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


TEST_CASE("WinAPI - Queue proxy", "[unit][gui]") {
    struct NotifierWindow {
        void postQueueNotification() {}
    };

    MockHandler<int> handler;
    QueueProxy<int, NotifierWindow> proxy(handler);

    REQUIRE_THROWS_AS(proxy.getQueue(), std::runtime_error);
    REQUIRE(proxy.handleMessage(SysMessage{0, 0, 0, 0}) == 0);
    REQUIRE(proxy.handleMessage(SysMessage{0, WM_APP_QUEUE, 0, 0}) == 0);
    REQUIRE(handler.debugQueue.empty());

    auto window = std::make_shared<NotifierWindow>();
    proxy.setNotifier(window);
    auto queue = proxy.getQueue();
    REQUIRE(proxy.handleMessage(SysMessage{0, 0, 0, 0}) == 0);
    REQUIRE(handler.debugQueue.empty());
    REQUIRE(proxy.handleMessage(SysMessage{0, WM_APP_QUEUE, 0, 0}) == 0);
    REQUIRE(handler.debugQueue.empty());

    queue->push(Message::GUIDestroyWindow<int>(42));
    REQUIRE(proxy.handleMessage(SysMessage{0, WM_APP_QUEUE, 0, 0}) == 0);
    REQUIRE(handler.debugQueue.take() == "destroy window 42");
    queue->push(Message::GUIExit());
    REQUIRE(proxy.handleMessage(SysMessage{0, WM_APP_QUEUE, 0, 0}) == 0);
    REQUIRE(handler.debugQueue.take() == "exit");
}


TEST_CASE("WinAPI - WindowMessageTranslator", "[unit][gui]") {
    MockHandler<int> handler;
    WindowMessageTranslator<int> translator(42, handler);
    REQUIRE(handler.debugQueue.empty());
    translator.onClose();
    REQUIRE(handler.debugQueue.take() == "window close 42");
}


/*
TEST_CASE("WinAPI - AppWindowManager", "[unit][gui]") {

}
*/

/*
TEST_CASE("WinAPI - Platform", "[unit][gui]") {
    using P = Platform<int>;
    auto appQueue = std::make_shared<AppQueue<int>>();
    GUICore<P> core(appQueue);
}
*/

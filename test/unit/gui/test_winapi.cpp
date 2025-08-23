// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/winapi/platform.hpp>
#include <gui/common/message.hpp>
#include <gui/common/core.hpp>

using namespace Istok::GUI;
using namespace Istok::GUI::WinAPI;

#include <cassert>
#include <memory>
#include <string>
#include <format>
#include <optional>
#include <thread>

namespace {

using DebugQueue = SyncWaitingQueue<std::string>;

template <typename WindowID>
class MockHandler: public GUIHandler<WindowID> {
public:
    DebugQueue debugQueue;
    
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

struct MockWindow {
    std::string title;
    std::unique_ptr<WindowTranslator> translator;

    MockWindow(const std::string& title = "") : title(title) {}
        
    void setTranslator(std::unique_ptr<WindowTranslator>&& value) {
        translator = std::move(value);
    }

    void removeTranslator() {
        translator.reset();
    }
};

}


TEST_CASE("WinAPI - Notifier", "[unit][gui]") {
    struct Window {
        int& counter;
        void postQueueNotification() noexcept { ++counter; }
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
        void postQueueNotification() noexcept {}
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


TEST_CASE("WinAPI - IDTranslator", "[unit][gui]") {
    MockHandler<int> handler;
    IDTranslator<int> translator(42, handler);
    REQUIRE(handler.debugQueue.empty());
    translator.onClose();
    REQUIRE(handler.debugQueue.take() == "window close 42");
}


TEST_CASE("WinAPI - AppWindowManager", "[unit][gui]") {
    MockHandler<int> handler;
    AppWindowManager<int, MockWindow> manager(handler);
    auto a = std::make_shared<MockWindow>();
    auto b = std::make_shared<MockWindow>();
    manager.attach(1, a);
    manager.attach(2, b);
    REQUIRE(a->translator != nullptr);
    REQUIRE(b->translator != nullptr);
    REQUIRE(handler.debugQueue.empty());
    a->translator->onClose();
    REQUIRE(handler.debugQueue.take() == "window close 1");
    b->translator->onClose();
    REQUIRE(handler.debugQueue.take() == "window close 2");
    REQUIRE(manager.release(2).get() == b.get());
    REQUIRE(b->translator == nullptr);
}


namespace {

class MockNotifierWindow {
public:
    MockNotifierWindow(MessageProxy& proxy)
        : proxy(proxy), instanceGetter(this) {}
    
    void postQueueNotification() noexcept {
        ++notifications;
    }
    
    SysResult handleQueueMessage() {
        assert(notifications > 0);
        --notifications;
        return proxy.handleMessage(SysMessage{0, WM_APP_QUEUE, 0, 0});
    }

    static MockNotifierWindow* release() {
        return InstanceGetter<MockNotifierWindow>::release();
    }

private:
    MessageProxy& proxy;

    InstanceGetter<MockNotifierWindow> instanceGetter;
    int notifications = 0;
};

}


TEST_CASE("WinAPI - QueueManager", "[unit][gui]") {
    MockHandler<int> handler;
    QueueManager<int, MockNotifierWindow> manager(handler);
    auto window = MockNotifierWindow::release();
    auto queue = manager.getQueue();
    queue->push(Message::GUIDestroyWindow<int>(42));
    window->handleQueueMessage();
    REQUIRE(handler.debugQueue.take() == "destroy window 42");
    queue->push(Message::GUIExit{});
    window->handleQueueMessage();
    REQUIRE(handler.debugQueue.take() == "exit");
}


namespace {

class MockSysWindowManager {
public:
    std::shared_ptr<DebugQueue> debugQueue;
    
    using Window = MockWindow;

    MockSysWindowManager()
        : instanceGetter(this),
        debugQueue(std::make_shared<DebugQueue>()) {}

    std::shared_ptr<Window> create(WindowParams params) {
        std::string title = params.title.value_or("");        
        debugQueue->push(std::format("create {}", title));
        return std::make_shared<Window>(title);
    }

    void remove(std::shared_ptr<Window> window) {
        debugQueue->push(std::format("remove {}", window->title));
    }

    void runMessageLoop() {
        debugQueue->push("run message loop");
    }

    void stopMessageLoop() noexcept {
        debugQueue->push("stop message loop");
    }

    static MockSysWindowManager* release() {
        return InstanceGetter<MockSysWindowManager>::release();
    }

private:
    InstanceGetter<MockSysWindowManager> instanceGetter;
};

}


TEST_CASE("WinAPI - WindowManager", "[unit][gui]") {
    MockHandler<int> handler;
    WindowManager<int, MockSysWindowManager> manager(handler);
    auto debugQueue = MockSysWindowManager::release()->debugQueue;
    manager.runMessageLoop();
    REQUIRE(debugQueue->take() == "run message loop");
    manager.newWindow(1, WindowParams{{}, "a"});
    REQUIRE(debugQueue->take() == "create a");
    manager.newWindow(2, WindowParams{{}, "b"});
    REQUIRE(debugQueue->take() == "create b");
    manager.destroyWindow(2);
    REQUIRE(debugQueue->take() == "remove b");
    manager.stopMessageLoop();
    REQUIRE(debugQueue->take() == "stop message loop");
}


TEST_CASE("WinAPI - Platform", "[unit][gui]") {
    MockHandler<int> handler;
    Platform<int, MockNotifierWindow, MockSysWindowManager> platform(handler);
    auto debugQueue = MockSysWindowManager::release()->debugQueue;
    auto queue = platform.getQueue();
    REQUIRE(queue != nullptr);
    platform.run();
    REQUIRE(debugQueue->take() == "run message loop");
    platform.newWindow(1, WindowParams{{}, "a"});
    REQUIRE(debugQueue->take() == "create a");
    platform.newWindow(2, WindowParams{{}, "b"});
    REQUIRE(debugQueue->take() == "create b");
    platform.destroyWindow(2);
    REQUIRE(debugQueue->take() == "remove b");
    platform.stop();
    REQUIRE(debugQueue->take() == "stop message loop");
}

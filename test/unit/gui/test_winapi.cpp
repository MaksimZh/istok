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
#include <optional>
#include <thread>
#include <semaphore>

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


TEST_CASE("WinAPI - IDTranslator", "[unit][gui]") {
    MockHandler<int> handler;
    IDTranslator<int> translator(42, handler);
    REQUIRE(handler.debugQueue.empty());
    translator.onClose();
    REQUIRE(handler.debugQueue.take() == "window close 42");
}


TEST_CASE("WinAPI - AppWindowManager", "[unit][gui]") {
    struct Window {
        std::unique_ptr<WindowTranslator> translator;
        
        void setTranslator(std::unique_ptr<WindowTranslator>&& value) {
            translator = std::move(value);
        }

        void removeTranslator() {
            translator.reset();
        }
    };
    
    MockHandler<int> handler;
    AppWindowManager<int, Window> manager(handler);
    auto a = std::make_shared<Window>();
    auto b = std::make_shared<Window>();
    manager.attach(1, a);
    manager.attach(2, b);
    REQUIRE(a->translator != nullptr);
    REQUIRE(b->translator != nullptr);
    REQUIRE(handler.debugQueue.empty());
    a->translator->onClose();
    REQUIRE(handler.debugQueue.take() == "window close 1");
    b->translator->onClose();
    REQUIRE(handler.debugQueue.take() == "window close 2");
}


namespace {

template <typename T>
class ImplicitInstanceGetter {
public:
    ImplicitInstanceGetter(T* self) : self(self) {
        std::unique_lock lock(mut);
        cv.wait(lock, [] { return instance == nullptr; });
        instance = self;
    }

    ~ImplicitInstanceGetter() {
        std::lock_guard lock(mut);
        if (instance == self) {
            instance = nullptr;
            cv.notify_all();
        }
    }

    static T* release() {
        std::lock_guard lock(mut);
        T* tmp = instance;
        instance = nullptr;
        cv.notify_all();
        return tmp;
    }

private:
    T* self;
    static T* instance;
    static std::mutex mut;
    static std::condition_variable cv;
};

template <typename T>
T* ImplicitInstanceGetter<T>::instance;

template <typename T>
std::mutex ImplicitInstanceGetter<T>::mut;

template <typename T>
std::condition_variable ImplicitInstanceGetter<T>::cv;


class MockNotifierWindow {
public:
    MockNotifierWindow(MessageProxy& proxy)
        : proxy(proxy), instanceGetter(this) {}
    
    void postQueueNotification() {
        ++notifications;
    }
    
    SysResult handleQueueMessage() {
        assert(notifications > 0);
        --notifications;
        return proxy.handleMessage(SysMessage{0, WM_APP_QUEUE, 0, 0});
    }

    static MockNotifierWindow* release() {
        return ImplicitInstanceGetter<MockNotifierWindow>::release();
    }

private:
    MessageProxy& proxy;

    ImplicitInstanceGetter<MockNotifierWindow> instanceGetter;
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

class MockWindow {};

class MockSysWindowManager {
public:
    using Window = MockWindow;
};

}


TEST_CASE("WinAPI - WindowManager", "[unit][gui]") {
    MockHandler<int> handler;
    WindowManager<int, MockSysWindowManager> manager(handler);
}


/*
TEST_CASE("WinAPI - Platform", "[unit][gui]") {
    using P = Platform<int>;
    auto appQueue = std::make_shared<AppQueue<int>>();
    GUICore<P> core(appQueue);
}
*/

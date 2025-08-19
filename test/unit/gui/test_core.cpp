// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/common/core.hpp>

using namespace Istok::Tools;
using namespace Istok::GUI;

#include <mutex>
#include <string>
#include <format>


namespace {

using AppQueue = SyncWaitingQueue<AppMessage<int>>;

/**
 * @brief Mock for GUI platform-dependent stuff
 * 
 * Pointer to instance can be obtained even if it is created implicitly.
 * Each instance holds its own debug message queue as a shared pointer.
 * Thus the queue is accessible even after instance destruction.
 * 
 */
class MockPlatform {
public:
    using InQueue = SyncWaitingQueue<GUIMessage<int>>;
    using DebugQueue = SyncWaitingQueue<std::string>;

    /**
     * @brief Debug message queue
     */
    std::shared_ptr<DebugQueue> debugQueue;

    /**
     * @brief Construct a new MockPlatform instance
     * 
     * The pointer to new instance can be obtained via static `release` method.
     * If there is already an instance that is not released yet the constructor
     * will wait.
     */
    MockPlatform()
        : queue(std::make_shared<InQueue>()),
        debugQueue(std::make_shared<DebugQueue>())
    {
        std::unique_lock lock(mut);
        debugQueue->push("create");
        cv.wait(lock, [] { return instance == nullptr; });
        instance = this;
    }

    /**
     * @brief Destroy the MockPlatform instance
     * 
     * Releases the instance on destruction so that other instances
     * can be created.
     */
    ~MockPlatform() {
        std::lock_guard lock(mut);
        debugQueue->push("destroy");
        if (instance == this) {
            instance = nullptr;
            cv.notify_all();
        }
    }

    /**
     * @brief Get pointer to last created instance
     * 
     * This method returns the pointer to the last created instance of
     * this class. Call of this method unlocks creation of other instances.
     * 
     * @return MockPlatform* 
     * 
     * @details
     * When instance is created all further constructor calls wait until
     * the pointer to this instance is obtained by call of this function
     * or until the destructor is called.
     * Creation of instances and obtaining pointers to them is thread-safe.
     * 
     * @usage
     * ... // some code creating instance
     * MockPlatform* instance = MockPlatform::release();
     */
    static MockPlatform* release() {
        std::lock_guard lock(mut);
        MockPlatform* tmp = instance;
        instance = nullptr;
        cv.notify_all();
        return tmp;
    }

    MockPlatform(const MockPlatform&) = delete;
    MockPlatform& operator=(const MockPlatform&) = delete;
    MockPlatform(MockPlatform&&) = delete;
    MockPlatform& operator=(MockPlatform&&) = delete;
    
    std::shared_ptr<InQueue> getInQueue() {
        return queue;
    }

    /**
     * @brief Make all preparations for run and return immediately
     * 
     * To be used for handler testing when the mock platform is managed
     * synchronously in the same thread.
     * 
     * @param handler The object used to handle messages that are not processed
     * by the platform itself
     */
    void startRun(WindowMessageHandler<int>& handler) {
        debugQueue->push("run");
        this->handler = &handler;
        running = true;
    }

    /**
     * @brief Run the message handling loop
     * 
     * @param handler The object used to handle messages that are not processed
     * by the platform itself
     */
    void run(WindowMessageHandler<int>& handler) {
        startRun(handler);
        while (running) {
            this->handler->onMessage(queue->take());
        }
    }

    void sendQueue(GUIMessage<int> msg) {
        handler->onMessage(msg);
    }

    void sendClose(int id) {
        handler->onClose(id);
    }

    /**
     * @brief Stop the message handling loop
     * 
     * Not thread-safe. Must be called only in the same thread with `run`.
     */
    void stop() {
        debugQueue->push("stop");
        running = false;
    }

    void newWindow(int id, WindowParams params) {
        debugQueue->push(std::format("new window {}", id));
    }

    void destroyWindow(int id) {
        debugQueue->push(std::format("destroy window {}", id));
    }

private:
    static MockPlatform* instance;
    static std::mutex mut;
    static std::condition_variable cv;

    WindowMessageHandler<int>* handler;
    std::shared_ptr<InQueue> queue;
    bool running;
};

MockPlatform* MockPlatform::instance = nullptr;
std::mutex MockPlatform::mut;
std::condition_variable MockPlatform::cv;

}


TEST_CASE("GUI - Handler", "[unit][gui]") {
    MockPlatform platform;
    std::shared_ptr<MockPlatform::DebugQueue> debugQueue =
        MockPlatform::release()->debugQueue;
    REQUIRE(debugQueue->take() == "create");
    std::shared_ptr<AppQueue> appQueue = std::make_shared<AppQueue>();
    Handler<int, MockPlatform, AppQueue> handler(platform, appQueue);
    platform.startRun(handler);
    REQUIRE(debugQueue->take() == "run");

    SECTION("exit") {
        platform.sendQueue(Message::GUIExit{});
        REQUIRE(debugQueue->take() == "stop");
    }

    SECTION("new window") {
        platform.sendQueue(Message::GUINewWindow<int>(42, WindowParams{}));
        REQUIRE(debugQueue->take() == "new window 42");
    }

    SECTION("destroy window") {
        platform.sendQueue(Message::GUIDestroyWindow<int>(42));
        REQUIRE(debugQueue->take() == "destroy window 42");
    }

    SECTION("on close") {
        platform.sendQueue(Message::GUINewWindow<int>(42, WindowParams{}));
        REQUIRE(debugQueue->take() == "new window 42");
        platform.sendClose(42);
        auto msg = appQueue->take();
        REQUIRE(std::holds_alternative<Message::AppWindowClosed<int>>(msg));
        REQUIRE(std::get<Message::AppWindowClosed<int>>(msg).id == 42);
    }
}


TEST_CASE("GUI - GUI", "[unit][gui]") {
    std::shared_ptr<MockPlatform::DebugQueue> debugQueue;
    {
        GUIFor<int, MockPlatform> gui;
        MockPlatform* platform = MockPlatform::release();
        debugQueue = platform->debugQueue;
        REQUIRE(debugQueue->take() == "create");
        REQUIRE(debugQueue->take() == "run");
        gui.newWindow(42, WindowParams{});
        REQUIRE(debugQueue->take() == "new window 42");
        gui.destroyWindow(42);
        REQUIRE(debugQueue->take() == "destroy window 42");
    }
    REQUIRE(debugQueue->take() == "stop");
    REQUIRE(debugQueue->take() == "destroy");
}

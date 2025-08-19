// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/common/core.hpp>

using namespace Istok::Tools;
using namespace Istok::GUI;

#include <mutex>
#include <string>
#include <format>
#include <variant>


namespace {

using DebugQueue = SyncWaitingQueue<std::string>;

/**
 * @brief Mock for GUI platform-dependent stuff
 * 
 * Pointer to instance can be obtained even if it is created implicitly.
 * Each instance holds its own debug message queue as a shared pointer.
 * Thus the queue is accessible even after instance destruction.
 * 
 */
template <typename WindowID_>
class MockPlatform {
public:
    using WindowID = WindowID_;
    using InQueue = SyncWaitingQueue<GUIMessage<WindowID>>;

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
     * MockPlatform<T>* instance = MockPlatform<T>::release();
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
    
    std::shared_ptr<InQueue> getQueue() noexcept {
        return queue;
    }

    void run(GUIHandler<WindowID>& handler) {
        debugQueue->push("run");
        this->handler = &handler;
        running = true;
        if (sync) {
            return;
        }
        while (running) {
            processQueue();
        }
    }

    /**
     * @brief Enter synchronous mode
     * 
     * Must be called before `run`.
     * 
     */
    void syncStart() {
        sync = true;
    }

    void syncProcessQueue() {
        assert(!queue->empty());
        processQueue();
    }

    void stop() {
        debugQueue->push("stop");
        running = false;
    }

    void newWindow(WindowID id, WindowParams params) {
        debugQueue->push(std::format("new window {}", id));
    }

    void destroyWindow(WindowID id) {
        debugQueue->push(std::format("destroy window {}", id));
    }

private:
    static MockPlatform* instance;
    static std::mutex mut;
    static std::condition_variable cv;

    GUIHandler<WindowID>* handler;
    std::shared_ptr<InQueue> queue;
    bool running = false;
    bool sync = false;

    void processQueue() {
        assert(this->handler);
        GUIMessage<WindowID> msg = queue->take();
        if (std::holds_alternative<Message::GUIExit>(msg)) {
            this->handler->onExit();
            return;
        }
    }
};

template <typename WindowID>
MockPlatform<WindowID>* MockPlatform<WindowID>::instance = nullptr;

template <typename WindowID>
std::mutex MockPlatform<WindowID>::mut;

template <typename WindowID>
std::condition_variable MockPlatform<WindowID>::cv;

}


TEST_CASE("GUI - Core", "[unit][gui]") {
    using Platform = MockPlatform<int>;
    auto appQueue = std::make_shared<AppQueue<int>>();
    GUICore<Platform> core(appQueue);
    auto platform = Platform::release();
    auto queue = core.getQueue();
    auto debugQueue = platform->debugQueue;
    REQUIRE(debugQueue->take() == "create");
    
    platform->syncStart();
    core.run();
    REQUIRE(debugQueue->take() == "run");

    SECTION("exit") {
        queue->push(Message::GUIExit{});
        platform->syncProcessQueue();
        REQUIRE(debugQueue->take() == "stop");
    }

    /*
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
    */
}

/*
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
*/
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <tools/threads.hpp>
#include <tools/queue.hpp>

using namespace Istok::Tools;

#include <memory>
#include <stdexcept>
#include <string>
#include <format>
#include <thread>
#include <future>
#include <chrono>
using namespace std::chrono_literals;

namespace {

using StringQueue = SyncWaitingQueue<std::string>;
using SharedStringQueue = std::shared_ptr<StringQueue>;

class MockCore {
public:
    MockCore(std::promise<SharedStringQueue> logQueue, bool success = true) {
        log = std::make_shared<StringQueue>();
        logQueue.set_value(log);
        log->push("create");
        if (!success) {
            throw std::runtime_error("constructor failed");
        }
        queue = std::make_shared<StringQueue>();
    }
    
    static std::string exitMessage() noexcept {
        return "exit";
    }
    
    SharedStringQueue getQueue() noexcept {
        return queue;
    }

    void run() noexcept {
        log->push("start");
        while (true) {
            std::string msg = queue->take();
            log->push(std::format("msg: {}", msg));
            if (msg == "exit") {
                break;
            }
        }
        log->push("finish");
    }

private:
    SharedStringQueue queue;
    SharedStringQueue log;
};

}


TEST_CASE("Tools - launcher", "[unit][tools]") {
    std::promise<SharedStringQueue> prom;
    std::future<SharedStringQueue> fut = prom.get_future();
    SharedStringQueue log;

    SECTION("normal") {
        {
            Launcher<MockCore> launcher(std::move(prom));
            log = fut.get();
            REQUIRE(log->take() == "create");
            REQUIRE(log->take() == "start");
            SharedStringQueue queue = launcher.getQueue();
            queue->push("foo");
            REQUIRE(log->take() == "msg: foo");
            queue->push("boo");
            REQUIRE(log->take() == "msg: boo");
        }
        REQUIRE(log->take() == "msg: exit");
        REQUIRE(log->take() == "finish");
    }
    
    SECTION("fail launch") {
        REQUIRE_THROWS_MATCHES(
            Launcher<MockCore>(std::move(prom), false),
            std::runtime_error,
            Catch::Matchers::Predicate<std::runtime_error>(
                [](const std::runtime_error& e) {
                    return std::string(e.what()) == "constructor failed";
                },
                "constructor must fail"
            )
        );
        log = fut.get();
        REQUIRE(log->take() == "create");
    }

    SECTION("emergency stop") {
        try {
            Launcher<MockCore> launcher(std::move(prom));
            log = fut.get();
            REQUIRE(log->take() == "create");
            REQUIRE(log->take() == "start");
            throw std::runtime_error("stop");
        } catch(...) {}
        REQUIRE(log->take() == "msg: exit");
        REQUIRE(log->take() == "finish");
    }
}


TEST_CASE("Tools - channel", "[unit][tools]") {
    using Queue = SyncWaitingQueue<int>;
    auto inQueue = std::make_shared<Queue>();
    auto outQueue = std::make_shared<Queue>();

    Channel<Queue, Queue> channel(inQueue, outQueue);

    SECTION("Synchronous push") {
        REQUIRE(outQueue->empty() == true);
        channel.push(1);
        REQUIRE(outQueue->take() == 1);
        channel.push(2);
        channel.push(3);
        REQUIRE(outQueue->take() == 2);
        REQUIRE(outQueue->take() == 3);
    }

    SECTION("Synchronous take") {
        REQUIRE(channel.empty() == true);
        inQueue->push(1);
        REQUIRE(channel.empty() == false);
        REQUIRE(channel.take() == 1);
        inQueue->push(2);
        inQueue->push(3);
        REQUIRE(channel.take() == 2);
        REQUIRE(channel.take() == 3);
    }

    SECTION("Asynchronous push") {
        std::thread thread([&]{
            for (int i = 0; i < 20; ++i) {
                std::this_thread::sleep_for(1ms);
                channel.push(i);
            }
        });
        for (int i = 0; i < 20; ++i) {
            std::this_thread::sleep_for(1ms);
            REQUIRE(outQueue->take() == i);
        }
        thread.join();
    }

    SECTION("Asynchronous take") {
        std::thread thread([&]{
            for (int i = 0; i < 20; ++i) {
                std::this_thread::sleep_for(1ms);
                inQueue->push(i);
            }
        });
        for (int i = 0; i < 20; ++i) {
            std::this_thread::sleep_for(1ms);
            REQUIRE(channel.take() == i);
        }
        thread.join();
    }
}

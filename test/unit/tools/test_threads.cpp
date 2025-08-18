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

class MockCore {
public:
    MockCore(std::promise<MockCore*> self) {
        queue = std::make_shared<StringQueue>();
        log = std::make_shared<StringQueue>();
        log->push("create");
        self.set_value(this);
    }
    
    static std::string exitMessage() {
        return "exit";
    }
    
    std::shared_ptr<StringQueue> getQueue() {
        return queue;
    }

    void run() {
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

    void onException(std::exception_ptr e) {
        log->push("exception");
    }

    std::shared_ptr<StringQueue> log;

private:
    std::shared_ptr<StringQueue> queue;
};

}


TEST_CASE("Tools - launcher - just exit", "[unit][tools]") {
    std::promise<MockCore*> prom;
    std::future<MockCore*> fut = prom.get_future();
    std::shared_ptr<StringQueue> log;
    {
        Launcher<MockCore> launcher(std::move(prom));
        log = fut.get()->log;
        REQUIRE(log->take() == "create");
        REQUIRE(log->take() == "start");
    }
    REQUIRE(log->take() == "msg: exit");
    REQUIRE(log->take() == "finish");
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

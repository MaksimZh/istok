// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <tools/threads.hpp>
#include <tools/queue.hpp>

using namespace Istok::Tools;

#include <thread>
#include <chrono>
using namespace std::chrono_literals;


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

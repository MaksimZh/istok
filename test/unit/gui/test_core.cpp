// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/core.hpp>

using namespace Istok::Tools;
using namespace Istok::GUI;

#include <memory>

TEST_CASE("GUI - channel", "[unit][gui]") {
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
}

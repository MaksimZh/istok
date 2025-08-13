// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <tools/queue.hpp>

#include <thread>
#include <memory>

namespace Istok::GUI {

class WindowMessageHandler {};

template <typename InQueue, typename OutQueue>
class Channel {};

template <typename Platform, typename InQueue, typename OutQueue>
class Core : WindowMessageHandler {
public:
    Core(std::shared_ptr<OutQueue> outQueue)
        : platform(*this),
        channel(platform.getInQueue(), outQueue) {}

    std::shared_ptr<CoreQueue> getInQueue() {
        return channel.getInQueue();
    }

private:
    Platform platform;
    Channel<InQueue, OutQueue> channel;
};

} // namespace Istok::GUI

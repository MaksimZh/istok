// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <tools/queue.hpp>

#include <thread>
#include <memory>

namespace Istok::GUI {

class WindowMessageHandler {};

template <typename InQueue, typename OutQueue>
class Channel {
public:
    Channel(
        std::shared_ptr<InQueue> inQueue,
        std::shared_ptr<OutQueue> outQueue
    ) : inQueue(inQueue), outQueue(outQueue)
    {
        assert(this->inQueue);
        assert(this->outQueue);
    }

    bool empty() {
        return inQueue->empty();
    }

    void push(OutQueue::element_type&& value) {
        outQueue->push(std::move(value));
    }

    void push(const OutQueue::element_type& value) {
        outQueue->push(value);
    }

    InQueue::element_type take() {
        return inQueue->take();
    }

private:
    std::shared_ptr<InQueue> inQueue;
    std::shared_ptr<OutQueue> outQueue;
};


template <typename Platform, typename InQueue, typename OutQueue>
class Core : WindowMessageHandler {
public:
    Core(std::shared_ptr<OutQueue> outQueue)
        : platform(*this),
        channel(platform.getInQueue(), outQueue) {}

    std::shared_ptr<InQueue> getInQueue() {
        return channel.getInQueue();
    }

private:
    Platform platform;
    Channel<InQueue, OutQueue> channel;
};

} // namespace Istok::GUI

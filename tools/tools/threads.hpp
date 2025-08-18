// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <memory>
#include <thread>
#include <future>

namespace Istok::Tools {

template <typename Core>
class Launcher {
public:
    using SharedQueue = decltype(std::declval<Core>().getQueue());

    template <typename... Args>
    Launcher(Args&&... args) {
        std::promise<SharedQueue> prom;
        std::future<SharedQueue> fut = prom.get_future();
        thread = std::thread(
            proc<Args...>,
            std::move(prom),
            std::forward<Args>(args)...);
        try {
            queue = fut.get();
        } catch(...) {
            thread.join();
            throw;
        }
    }

    ~Launcher() {
        queue->push(Core::exitMessage());
        thread.join();
    }
    
    SharedQueue getQueue() {
        return queue;
    }

private:
    SharedQueue queue;
    std::thread thread;

    template <typename... Args>
    static void proc(std::promise<SharedQueue> queue, Args&&... args) {
        std::unique_ptr<Core> core;
        try {
            core = std::make_unique<Core>(std::forward<Args>(args)...);
            queue.set_value(core->getQueue());
        } catch (...) {
            queue.set_exception(std::current_exception());
        }
        try {
            core->run();
        } catch (...) {
            core->onException(std::current_exception());
        }
    }
};


template <typename InQueue, typename OutQueue>
class Channel {
public:
    using InType = decltype(std::declval<InQueue>().take());
    using OutType = decltype(std::declval<OutQueue>().take());

    Channel() = default;
    
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

    void push(OutType&& value) {
        outQueue->push(std::move(value));
    }

    void push(const OutType& value) {
        outQueue->push(value);
    }

    InType take() {
        return inQueue->take();
    }

private:
    std::shared_ptr<InQueue> inQueue;
    std::shared_ptr<OutQueue> outQueue;
};

} // namespace Istok::Tools

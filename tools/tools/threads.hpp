// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <memory>
#include <thread>
#include <future>

namespace Istok::Tools {

template <typename Core>
concept ThreadCore = requires(Core core) {
    {Core::exitMessage()} noexcept;
    {core.getQueue()};
    {core.run()} noexcept -> std::same_as<void>;
    {core.getQueue()->push(Core::exitMessage())} noexcept -> std::same_as<void>;
};

template <ThreadCore Core>
class Launcher {
public:
    using SharedQueue = decltype(std::declval<Core>().getQueue());

    template <typename... Args>
        requires std::constructible_from<Core, Args...>
    Launcher(Args... args) {
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

    Launcher(const Launcher&) = delete;
    Launcher& operator=(const Launcher&) = delete;
    Launcher(Launcher&&) = delete;
    Launcher& operator=(Launcher&&) = delete;
    
    SharedQueue getQueue() {
        return queue;
    }

private:
    std::thread thread;
    SharedQueue queue;

    template <typename... Args>
    static void proc(std::promise<SharedQueue> queue, Args... args) {
        std::unique_ptr<Core> core;
        try {
            core = std::make_unique<Core>(std::forward<Args>(args)...);
            queue.set_value(core->getQueue());
        } catch (...) {
            queue.set_exception(std::current_exception());
            return;
        }
        core->run();
    }
};


template <typename InQueue, typename OutQueue>
class Channel {
public:
    using InType = decltype(std::declval<InQueue>().take());
    using OutType = decltype(std::declval<OutQueue>().take());
    
    Channel(
        std::shared_ptr<InQueue> inQueue,
        std::shared_ptr<OutQueue> outQueue
    ) : inQueue(inQueue), outQueue(outQueue)
    {
        if (!this->inQueue || !this->outQueue) {
            throw std::runtime_error("Uninitialized queues");
        }
    }

    bool empty() noexcept {
        return inQueue->empty();
    }

    void push(OutType&& value) noexcept {
        outQueue->push(std::move(value));
    }

    void push(const OutType& value) noexcept {
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

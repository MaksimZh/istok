// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <tools/queue.hpp>

#include <thread>
#include <future>
#include <memory>

namespace Istok::GUI {

class GUIMessage {};
class AppMessage {};

class WindowMessageHandler {};


template <typename Platform, typename AppQueue>
class Core : WindowMessageHandler {
public:
    Core(Platform& platform, std::shared_ptr<AppQueue> appQueue)
        : platform(platform), appQueue(appQueue) {}

private:
    Platform& platform;
    AppQueue appQueue;
};


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

template <typename Notifier>
using GUIQueue = Tools::SyncNotifyingQueue<GUIMessage, Notifier>;

using AppQueue = Tools::SyncWaitingQueue<AppMessage>;


template <typename Platform>
class GUIFor {
public:
    using GUIQueueType = GUIQueue<Platform::Notifier>;
    using SharedGUIQueue = std::shared_ptr<GUIQueueType>;
    
    GUIFor() {
        std::shared_ptr<AppQueue> appQueue;
        std::promise<SharedGUIQueue> guiQueuePromise;
        std::future<SharedGUIQueue> guiQueueFuture = guiQueuePromise.get_future();
        thread = std::thread(proc, std::move(guiQueuePromise), appQueue);
        channel = Channel(guiQueueFuture.get(), appQueue);
    }

    ~GUIFor() {
        thread.join();
    }

    GUIFor(const GUIFor&) = delete;
    GUIFor& operator=(const GUIFor&) = delete;
    GUIFor(GUIFor&&) = delete;
    GUIFor& operator=(GUIFor&&) = delete;

private:
    std::thread thread;
    Channel<GUIQueueType, AppQueue> channel;

    static void proc(
        std::promise<SharedGUIQueue> guiQueuePromise,
        std::shared_ptr<AppQueue> appQueue)
    {
        Platform platform;
        guiQueuePromise.set_value(platform.getNotifier());
        Core core(platform, appQueue);
        platform.setMessageHandler(core);
    }
};


} // namespace Istok::GUI

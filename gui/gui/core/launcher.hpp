// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <tools/queue.hpp>
#include "message.hpp"

#include <type_traits>
#include <thread>
#include <future>
#include <memory>

namespace Istok::GUI {

template <typename Platform, typename AppQueue>
class Handler : public WindowMessageHandler {
public:
    Handler(Platform& platform, std::shared_ptr<AppQueue> appQueue)
        : platform(platform), appQueue(appQueue) {}

    void handleMessage(GUIMessage msg) {
        if (std::holds_alternative<Message::GUIExit>(msg)) {
            platform.stop();
            return;
        }
    }

private:
    Platform& platform;
    std::shared_ptr<AppQueue> appQueue;
};


using AppQueue = Tools::SyncWaitingQueue<AppMessage>;


template <typename Platform>
class GUIFor {
public:
    using GUIQueue = Platform::InQueue;
    using SharedGUIQueue = std::shared_ptr<GUIQueue>;
    using SharedAppQueue = std::shared_ptr<AppQueue>;
    
    GUIFor() {
        SharedAppQueue appQueue = std::make_shared<AppQueue>();
        std::promise<SharedGUIQueue> guiQueuePromise;
        std::future<SharedGUIQueue> guiQueueFuture = guiQueuePromise.get_future();
        thread = std::thread(proc, std::move(guiQueuePromise), appQueue);
        channel = Tools::Channel(appQueue, guiQueueFuture.get());
    }

    ~GUIFor() {
        channel.push(Message::GUIExit{});
        thread.join();
    }

    GUIFor(const GUIFor&) = delete;
    GUIFor& operator=(const GUIFor&) = delete;
    GUIFor(GUIFor&&) = delete;
    GUIFor& operator=(GUIFor&&) = delete;

private:
    Tools::Channel<AppQueue, GUIQueue> channel;
    std::thread thread;

    static void proc(
        std::promise<SharedGUIQueue> guiQueue,
        SharedAppQueue appQueue)
    {
        assert(appQueue);
        Platform platform;
        guiQueue.set_value(platform.getInQueue());
        Handler handler(platform, appQueue);
        platform.run(handler);
    }
};

} // namespace Istok::GUI

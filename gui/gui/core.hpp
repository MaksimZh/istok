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
class Core : public WindowMessageHandler {
public:
    Core(Platform& platform, std::shared_ptr<AppQueue> appQueue)
        : platform(platform), appQueue(appQueue) {}

private:
    Platform& platform;
    std::shared_ptr<AppQueue> appQueue;
};


template <typename Notifier>
using GUIQueue = Tools::SyncNotifyingQueue<GUIMessage, Notifier>;

using AppQueue = Tools::SyncWaitingQueue<AppMessage>;


template <typename Platform>
class GUIFor {
public:
    using GUIQueueType = GUIQueue<typename Platform::Notifier>;
    using SharedGUIQueue = std::shared_ptr<GUIQueueType>;
    
    GUIFor() {
        std::shared_ptr<AppQueue> appQueue = std::make_shared<AppQueue>();
        std::promise<SharedGUIQueue> guiQueuePromise;
        std::future<SharedGUIQueue> guiQueueFuture = guiQueuePromise.get_future();
        thread = std::thread(proc, std::move(guiQueuePromise), appQueue);
        channel = Tools::Channel(guiQueueFuture.get(), appQueue);
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
    Tools::Channel<GUIQueueType, AppQueue> channel;

    static void proc(
        std::promise<SharedGUIQueue> guiQueuePromise,
        std::shared_ptr<AppQueue> appQueue)
    {
        Platform platform;
        SharedGUIQueue guiQueue = std::make_shared<GUIQueueType>(platform.getNotifier());
        guiQueuePromise.set_value(guiQueue);
        Core core(platform, appQueue);
        platform.setMessageHandler(core);
    }
};


} // namespace Istok::GUI

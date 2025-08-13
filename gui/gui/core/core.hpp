// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <tools/queue.hpp>

#include <type_traits>
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


using AppQueue = Tools::SyncWaitingQueue<AppMessage>;


template <typename Platform>
class PlatformGUI {
public:
    using GUIQueue = Platform::InQueue;
    using SharedGUIQueue = std::shared_ptr<GUIQueue>;
    
    PlatformGUI() {
        std::shared_ptr<AppQueue> appQueue = std::make_shared<AppQueue>();
        std::promise<SharedGUIQueue> guiQueuePromise;
        std::future<SharedGUIQueue> guiQueueFuture = guiQueuePromise.get_future();
        thread = std::thread(proc, std::move(guiQueuePromise), appQueue);
        channel = Tools::Channel(guiQueueFuture.get(), appQueue);
    }

    ~PlatformGUI() {
        thread.join();
    }

    PlatformGUI(const PlatformGUI&) = delete;
    PlatformGUI& operator=(const PlatformGUI&) = delete;
    PlatformGUI(PlatformGUI&&) = delete;
    PlatformGUI& operator=(PlatformGUI&&) = delete;

private:
    std::thread thread;
    Tools::Channel<GUIQueue, AppQueue> channel;

    static void proc(
        std::promise<SharedGUIQueue> guiQueuePromise,
        std::shared_ptr<AppQueue> appQueue)
    {
        Platform platform;
        guiQueuePromise.set_value(platform.getInQueue());
        Core core(platform, appQueue);
        platform.setMessageHandler(core);
    }
};


} // namespace Istok::GUI

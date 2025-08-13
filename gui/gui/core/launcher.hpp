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
class GUIFor {
public:
    using GUIQueue = Platform::InQueue;
    using SharedGUIQueue = std::shared_ptr<GUIQueue>;
    
    GUIFor() {
        std::unique_ptr<Platform> platform = std::make_unique<Platform>();
        std::shared_ptr<AppQueue> appQueue = std::make_shared<AppQueue>();
        std::shared_ptr<GUIQueue> guiQueue = platform->getInQueue();
        channel = Tools::Channel(appQueue, guiQueue);
        thread = std::thread(proc, std::move(platform), appQueue);
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
        std::unique_ptr<Platform>&& platform,
        std::shared_ptr<AppQueue> appQueue)
    {
        assert(platform);
        assert(appQueue);
        Core core(*platform, appQueue);
        platform->setMessageHandler(core);
    }
};

} // namespace Istok::GUI

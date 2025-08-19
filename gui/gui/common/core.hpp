// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <tools/queue.hpp>
#include <tools/threads.hpp>
#include "message.hpp"

#include <type_traits>
#include <memory>

namespace Istok::GUI {

template <typename Platform>
class GUICore : public GUIHandler<typename Platform::WindowID> {
public:
    using WindowID = Platform::WindowID;

    GUICore(SharedAppQueue<WindowID> appQueue) : appQueue(appQueue) {}

    GUICore(const GUICore&) = delete;
    GUICore& operator=(const GUICore&) = delete;
    GUICore(GUICore&&) = delete;
    GUICore& operator=(GUICore&&) = delete;
    
    static GUIMessage<WindowID> exitMessage() noexcept {
        return Message::GUIExit{};
    }
    
    auto getQueue() noexcept {
        return platform.getQueue();
    }

    void run() noexcept {
        try {
            platform.run(*this);
        } catch (...) {
            appQueue->push(Message::AppGUIException(std::current_exception()));
        }
    }


    void onExit() noexcept override {
        platform.stop();
    }

private:
    Platform platform;
    SharedAppQueue<WindowID> appQueue;
};


template <typename Platform>
class GUIFor {
public:
    using WindowID = Platform::WindowID;
    
    GUIFor() : GUIFor(std::make_shared<AppQueue<WindowID>>()) {}

    GUIFor(const GUIFor&) = delete;
    GUIFor& operator=(const GUIFor&) = delete;
    GUIFor(GUIFor&&) = delete;
    GUIFor& operator=(GUIFor&&) = delete;

    void newWindow(WindowID id, WindowParams params) {
        channel.push(Message::GUINewWindow<WindowID>(id, params));
    }

    void destroyWindow(WindowID id) {
        channel.push(Message::GUIDestroyWindow<WindowID>(id));
    }

private:
    GUIFor(SharedAppQueue<WindowID> appQueue)
        : launcher(appQueue), channel(appQueue, launcher.getQueue()) {}

    Tools::Launcher<GUICore<Platform>> launcher;
    Tools::Channel<AppQueue<WindowID>, typename Platform::InQueue> channel;
};

} // namespace Istok::GUI

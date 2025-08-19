// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <tools/queue.hpp>
#include <tools/threads.hpp>
#include "message.hpp"

#include <type_traits>
#include <concepts>
#include <memory>

namespace Istok::GUI {

template <typename Platform>
concept GUIPlatform = requires {
    typename Platform::WindowID;
    requires std::default_initializable<Platform>;
} && requires(Platform platform) {
    {platform.getQueue()} noexcept;
    platform.run(std::declval<GUIHandler<typename Platform::WindowID>&>());
    {platform.stop()} noexcept;
    requires requires(Platform::WindowID id) {
        platform.newWindow(id, std::declval<WindowParams>());
        platform.destroyWindow(id);
    };
};


template <GUIPlatform Platform>
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


    void appExit() noexcept override {
        platform.stop();
    }

    void appNewWindow(WindowID id, WindowParams params) noexcept override {
        try {
            platform.newWindow(id, params);
        } catch(...) {
            appQueue->push(Message::AppGUIException(std::current_exception()));
        }
    }

    void appDestroyWindow(WindowID id) noexcept override {
        try {
            platform.destroyWindow(id);
        } catch(...) {
            appQueue->push(Message::AppGUIException(std::current_exception()));
        }
    }

    void sysCloseWindow(WindowID id) noexcept override {
        appQueue->push(Message::AppWindowClosed(id));
    }

private:
    Platform platform;
    SharedAppQueue<WindowID> appQueue;
};


template <GUIPlatform Platform>
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

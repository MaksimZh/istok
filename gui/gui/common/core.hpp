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
} && requires(Platform platform, GUIHandler<typename Platform::WindowID> handler) {
    Platform(handler);
    platform.getQueue();
    {platform.run()} -> std::same_as<void>;
    {platform.stop()} noexcept -> std::same_as<void>;
    requires requires(Platform::WindowID id) {
        {
            platform.newWindow(id, std::declval<WindowParams>())
        } -> std::same_as<void>;
        {platform.destroyWindow(id)} -> std::same_as<void>;
    };
};


template <GUIPlatform Platform>
class GUICore : public GUIHandler<typename Platform::WindowID> {
public:
    using WindowID = Platform::WindowID;

    GUICore(SharedAppQueue<WindowID> appQueue)
        : platform(*this), appQueue(appQueue) {}

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
            platform.run();
        } catch (...) {
            appQueue->push(Message::AppGUIException(std::current_exception()));
        }
    }

    void onMessage(GUIMessage<WindowID> msg) noexcept override {
        if (std::holds_alternative<Message::GUIExit>(msg)) {
            platform.stop();
            return;
        }
        try {
            onGUIMessage(msg);
            return;
        } catch(...) {
            appQueue->push(Message::AppGUIException(std::current_exception()));
        }
    }
    
    void onWindowClose(WindowID id) noexcept override {
        appQueue->push(Message::AppWindowClosed(id));
    }

private:
    Platform platform;
    SharedAppQueue<WindowID> appQueue;

    void onGUIMessage(GUIMessage<WindowID> msg) {
        if (std::holds_alternative<Message::GUINewWindow<WindowID>>(msg)) {
            auto message = std::get<Message::GUINewWindow<WindowID>>(msg);
            platform.newWindow(message.id, message.params);
            return;
        }
        if (std::holds_alternative<Message::GUIDestroyWindow<WindowID>>(msg)) {
            auto message = std::get<Message::GUIDestroyWindow<WindowID>>(msg);
            platform.destroyWindow(message.id);
            return;
        }
    }
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

    AppMessage<WindowID> getMessage() {
        return channel.take();
    }

private:
    GUIFor(SharedAppQueue<WindowID> appQueue)
        : launcher(appQueue), channel(appQueue, launcher.getQueue()) {}

    Tools::Launcher<GUICore<Platform>> launcher;
    using GUIQueue = std::decay_t<decltype(*launcher.getQueue())>;
    Tools::Channel<AppQueue<WindowID>, GUIQueue> channel;
};

} // namespace Istok::GUI

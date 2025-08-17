// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <tools/queue.hpp>
#include "message.hpp"

#include <type_traits>
#include <thread>
#include <future>
#include <memory>
#include <variant>

namespace Istok::GUI {

template <typename ID, typename Platform, typename AppQueue>
class Handler : public WindowMessageHandler<ID> {
public:
    Handler(Platform& platform, std::shared_ptr<AppQueue> appQueue)
        : platform(platform), appQueue(appQueue) {}

    void onMessage(GUIMessage<ID> msg) {
        if (std::holds_alternative<Message::GUIExit>(msg)) {
            platform.stop();
            return;
        }
        if (std::holds_alternative<Message::GUINewWindow<ID>>(msg)) {
            Message::GUINewWindow<ID> message =
                std::get<Message::GUINewWindow<ID>>(msg);
            platform.newWindow(message.id, message.params);
            return;
        }
        if (std::holds_alternative<Message::GUIDestroyWindow<ID>>(msg)) {
            platform.destroyWindow(
                std::get<Message::GUIDestroyWindow<ID>>(msg).id);
            return;
        }
    }

    void onClose(ID id) {
        appQueue->push(Message::AppWindowClosed<ID>(id));
    }

private:
    Platform& platform;
    std::shared_ptr<AppQueue> appQueue;
};


template <typename ID, typename Platform>
class GUIFor {
public:
    using GUIQueue = Platform::InQueue;
    using AppQueue = Tools::SyncWaitingQueue<AppMessage<ID>>;
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

    void newWindow(ID id, WindowParams params) {
        channel.push(Message::GUINewWindow<ID>(id, params));
    }

    void destroyWindow(ID id) {
        channel.push(Message::GUIDestroyWindow<ID>(id));
    }

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
        Handler<ID, Platform, AppQueue> handler(platform, appQueue);
        platform.run(handler);
    }
};


template <typename Core>
class Launcher {
public:
    using Channel = Tools::Channel<Core::InQueue, Core::OutQueue>;
    
    Launcher(std::shared_ptr<Core::OutQueue> outQueue) {
        std::promise<std::shared_ptr<Core::InQueue>> inQueuePromise;
        std::future<std::shared_ptr<Core::InQueue>> inQueueFuture =
            inQueuePromise.get_future();
        thread = std::thread(proc, std::move(inQueuePromise), outQueue);
        channel = Tools::Channel(outQueue, inQueueFuture.get());
    }

    ~Launcher() {
        channel.push(Core::ExitInMessage{});
        thread.join();
    }

    bool empty() {
        return channel.empty();
    }

    void push(Channel::OutType&& value) {
        channel.push(std::move(value));
    }

    void push(const Channel::OutType& value) {
        channel.push(value);
    }

    Channel::InType take() {
        return channel.take();
    }

private:
    Channel channel;
    std::thread thread;

    static void proc(
        std::promise<std::shared_ptr<Core::InQueue>> inQueue,
        std::shared_ptr<Core::OutQueue> outQueue,
    ) {
        try {
            Core core(outQueue);
            inQueue.set_value(core.getInQueue());
            core.run();
        } catch (...) {
            outQueue.push(Core::ErrorOutMessage(std::current_exception()));
        }
    }
};

} // namespace Istok::GUI

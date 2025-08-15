// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <tools/queue.hpp>
#include <gui/core/message.hpp>
#include <gui/winapi/window.hpp>

#include <windows.h>

#include <memory>


using namespace Istok::Tools;

namespace Istok::GUI::WinAPI {

class Window {
public:
    void postQueueNotification() {}
};


template <typename Queue>
class Translator {
public:
    Translator() {}
    
    void setQueue(std::shared_ptr<Queue> value) {
        assert(queue == nullptr);
        queue = value;
    }

    std::shared_ptr<Queue> getQueue() {
        return queue;
    }

private:
    std::shared_ptr<Queue> queue;
};


class Notifier {
public:
    Notifier(std::shared_ptr<Window> target) : target(target) {}

    Notifier(const Notifier&) = default;
    Notifier& operator=(const Notifier&) = default;
    Notifier(Notifier&&) = default;
    Notifier& operator=(Notifier&&) = default;

    void operator()() {
        target->postQueueNotification();
    }

private:
    std::shared_ptr<Window> target;
};


template <typename ID>
class Platform {
public:
    using InQueue = SyncNotifyingQueue<GUIMessage<ID>, Notifier>;

    Platform() {
        std::shared_ptr<Window> sampleWindow =
            std::make_shared<Window>(translator);
        translator.setQueue(std::make_shared<InQueue>(Notifier(sampleWindow)));
    }

    Platform(const Platform&) = delete;
    Platform& operator=(const Platform&) = delete;
    Platform(Platform&&) = delete;
    Platform& operator=(Platform&&) = delete;
    
    std::shared_ptr<InQueue> getInQueue() {
        return translator->getQueue();
    }

    void run(WindowMessageHandler<int>& handler) {
        while (true) {
            MSG msg;
            GetMessage(&msg, NULL, 0, 0);
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void stop() {
        PostQuitMessage(0);
    }

    void newWindow(int id, WindowParams params) {}

    void destroyWindow(int id) {}

private:
    Translator<InQueue> translator;
};

} // namespace Istok::GUI::WinAPI

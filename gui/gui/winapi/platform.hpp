// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <tools/queue.hpp>
#include <gui/core/message.hpp>

#include <windows.h>

#include <memory>


using namespace Istok::Tools;

namespace Istok::GUI::WinAPI {

template <typename Queue>
class Translator {
public:
    Translator(std::shared_ptr<Queue> queue) : queue(queue) {}
    
    std::shared_ptr<Queue> getQueue() {
        return queue;
    }

private:
    std::shared_ptr<Queue> queue;
};


class Notifier {};


template <typename ID>
class Platform {
public:
    using InQueue = SyncNotifyingQueue<GUIMessage<ID>, Notifier>;

    Platform() {}

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
    std::unique_ptr<Translator<InQueue>> translator;
};

} // namespace Istok::GUI::WinAPI

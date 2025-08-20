// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <tools/queue.hpp>
#include <gui/common/message.hpp>
#include "window.hpp"

#include <windows.h>

#include <memory>


using namespace Istok::Tools;

namespace Istok::GUI::WinAPI {

class Notifier {};

template <typename WindowID>
class QueueManager {
public:
    std::shared_ptr<SyncNotifyingQueue<GUIMessage<WindowID>, Notifier>> getQueue() noexcept {
        return nullptr;
    }
};


template <typename WindowID>
class WindowManager {
public:
    void newWindow(WindowID id, WindowParams params) {}
    void destroyWindow(WindowID id) {}
};


template <typename WindowID_>
class Platform {
public:
    using WindowID = WindowID_;

    Platform(GUIHandler<WindowID>& handler) {}

    Platform(const Platform&) = delete;
    Platform& operator=(const Platform&) = delete;
    Platform(Platform&&) = delete;
    Platform& operator=(Platform&&) = delete;
    
    auto getQueue() noexcept {
        return queueManager.getQueue();
    }

    void run() {
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

    void stop() noexcept {
        PostQuitMessage(0);
    }

    void newWindow(WindowID id, WindowParams params) {
        windowManager.newWindow(id, params);
    }

    void destroyWindow(WindowID id) {
        windowManager.destroyWindow(id);
    }

private:
    QueueManager<WindowID> queueManager;
    WindowManager<WindowID> windowManager;
};

} // namespace Istok::GUI::WinAPI

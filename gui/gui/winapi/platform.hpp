// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <tools/queue.hpp>
#include <gui/common/message.hpp>
#include <gui/winapi/window.hpp>

#include <windows.h>

#include <memory>


using namespace Istok::Tools;

namespace Istok::GUI::WinAPI {

class Notifier {};

template <typename WindowID_>
class WinPlatform {
public:
    using WindowID = WindowID_;
    using InQueue = SyncNotifyingQueue<GUIMessage<WindowID>, Notifier>;

    WinPlatform() {}

    WinPlatform(const WinPlatform&) = delete;
    WinPlatform& operator=(const WinPlatform&) = delete;
    WinPlatform(WinPlatform&&) = delete;
    WinPlatform& operator=(WinPlatform&&) = delete;
    
    std::shared_ptr<InQueue> getQueue() noexcept {
        return nullptr;
    }

    void run(GUIHandler<WindowID>& handler) {
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

    void newWindow(int id, WindowParams params) {}

    void destroyWindow(int id) {}

private:
};

} // namespace Istok::GUI::WinAPI

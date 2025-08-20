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


template <typename Handle, typename Window>
class WindowMapping {};


template <typename WindowID, typename Handle, typename Window>
class WindowStorage: public WindowMapping<Handle, Window> {};


template <typename Factory>
concept WindowFactory = requires {
    typename Factory::Handle;
    typename Factory::Window;
    typename Factory::Window::ID;
} && requires(WindowMapping<
    typename Factory::Handle,
    typename Factory::Window> mapping,
    GUIHandler<typename Factory::Window::ID> handler
) {
    Factory(mapping, handler);
} && requires(Factory factory, WindowParams params) {
    {factory.create(params)} -> std::same_as<typename Factory::Window>;
};


template <WindowFactory Factory>
class WindowManager {
public:
    using WindowID = typename Factory::Window::ID;
    
    WindowManager(GUIHandler<WindowID>& handler)
        : factory(storage, handler) {}
    
    void newWindow(WindowID id, WindowParams params) {
        auto [handle, window] = factory.create(params);
        storage.add(id, handle, window);
    }
    
    void destroyWindow(WindowID id) {
        storage.remove(id);
    }

private:
    WindowStorage<WindowID, Factory::Handle, Factory::Window> storage;
    Factory factory;
};


template <WindowFactory Factory>
class Platform {
public:
    using WindowID = Factory::Window::ID;

    Platform(GUIHandler<WindowID>& handler)
        : windowManager(handler) {}

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
    WindowManager<WindowID, Factory> windowManager;
};


class WindowProxy {};


} // namespace Istok::GUI::WinAPI

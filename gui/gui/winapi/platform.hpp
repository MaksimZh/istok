// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <tools/helpers.hpp>
#include <tools/queue.hpp>
#include <gui/common/message.hpp>

#include <windows.h>

#include <memory>
#include <unordered_map>


using namespace Istok::Tools;

namespace Istok::GUI::WinAPI {

struct SysMessage {
    HWND hWnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
};

using SysResult = LRESULT;

SysResult handleByDefault(SysMessage message) {
    return DefWindowProc(
        message.hWnd,
        message.msg,
        message.wParam,
        message.lParam
    );
}

class MessageProxy {
public:
    virtual SysResult handleMessage(SysMessage message) noexcept = 0;
};

constexpr UINT WM_APP_QUEUE = WM_APP + 1;


template <typename NotifierWindow>
class Notifier {
public:
    Notifier(std::shared_ptr<NotifierWindow> window) : target(window) {}

    void operator()() {
        if (std::shared_ptr<NotifierWindow> window = target.lock()) {
            window->postQueueNotification();
        }
    }

private:
    std::weak_ptr<NotifierWindow> target;
};


template <typename WindowID, typename NotifierWindow>
class QueueProxy: public MessageProxy {
public:
    using Queue = SyncNotifyingQueue<
        GUIMessage<WindowID>,
        Notifier<NotifierWindow>>;
    
    QueueProxy(GUIHandler<WindowID>& handler)
        :handler(handler) {}

    void setNotifier(std::shared_ptr<NotifierWindow> window) {
        queue = std::make_shared<Queue>(Notifier(window));
    }

    std::shared_ptr<Queue> getQueue() {
        if (queue == nullptr) {
            throw std::runtime_error("Notifier is not set");
        }
        return queue;
    }

    SysResult handleMessage(SysMessage message) noexcept {
        if (message.msg != WM_APP_QUEUE) {
            return handleByDefault(message);
        }
        if (!queue || queue->empty()) {
            return 0;
        }
        handler.onMessage(queue->take());
        return 0;
    }

private:
    GUIHandler<WindowID>& handler;
    std::shared_ptr<Queue> queue;
};


class WindowTranslator {
public:
    virtual void onClose() noexcept = 0;
};

template <typename WindowID>
class IDTranslator: public WindowTranslator {
public:
    IDTranslator(WindowID id, GUIHandler<WindowID>& handler)
        : id(id), handler(&handler) {}

    void onClose() noexcept override {
        handler->onWindowClose(id);
    }

private:
    WindowID id;
    GUIHandler<WindowID>* handler;
};


template <typename WindowID, typename Window>
class AppWindowManager {
public:
    AppWindowManager(GUIHandler<WindowID>& handler)
        : handler(handler) {}

    void attach(WindowID id, std::shared_ptr<Window> window) {
        if (storage.contains(id)) {
            throw std::runtime_error("Duplicate window id");
        }
        storage[id] = window;
        window->setTranslator(
            std::make_unique<IDTranslator<WindowID>>(id, handler));
    }
    
    std::shared_ptr<Window> release(WindowID id) {
        if (!storage.contains[id]) {
            throw std::runtime_error("Window not found by id");
        }
        std::shared_ptr<Window> window = storage[id];
        storage.erase(id);
        window->removeTranslator();
        return window;
    }

private:
    GUIHandler<WindowID>& handler;
    std::unordered_map<
        WindowID,
        std::shared_ptr<Window>,
        Istok::Tools::hash<WindowID>
    > storage; //TODO: wrap
};


template <typename WindowID, typename NotifierWindow>
class QueueManager {
public:
    QueueManager(GUIHandler<WindowID>& handler)
        : proxy(handler),
        window(std::make_shared<NotifierWindow>(proxy))
    {
        proxy.setNotifier(window);
    }

    auto getQueue() {
        return proxy.getQueue();
    }

private:
    std::shared_ptr<NotifierWindow> window;
    QueueProxy<WindowID, NotifierWindow> proxy;
};


template <typename WindowID, typename SysWindowManager>
class WindowManager {
public:
    using Window = SysWindowManager::Window;
    
    WindowManager(GUIHandler<WindowID>& handler)
        : appManager(handler) {}
    
    void newWindow(WindowID id, WindowParams params) {
        std::shared_ptr<Window> window = sysManager.create(params);
        appManager.attach(id, window);
    }
    
    void destroyWindow(WindowID id) {
        std::shared_ptr<Window> window = appManager.release(id);
        sysManager.destroy(window);
    }

private:
    SysWindowManager sysManager;
    AppWindowManager<WindowID, Window> appManager;
};


/*
template <typename WindowID_, typename NotifierWindow, typename SysWindowManager>
class Platform {
public:
    using WindowID = WindowID_i;

    Platform(GUIHandler<WindowID>& handler)
        : queueManager(handler), windowManager(handler) {}

    Platform(const Platform&) = delete;
    Platform& operator=(const Platform&) = delete;
    Platform(Platform&&) = delete;
    Platform& operator=(Platform&&) = delete;
    
    auto getQueue() {
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
    QueueManager<WindowID, NotifierWindow> queueManager;
    WindowManager<WindowID, SysWindowManager> windowManager;
};
*/

} // namespace Istok::GUI::WinAPI

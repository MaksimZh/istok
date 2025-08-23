// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <tools/helpers.hpp>
#include <tools/queue.hpp>
#include <gui/common/message.hpp>

#include <windows.h>

#include <memory>
#include <unordered_map>
#include <optional>


using namespace Istok::Tools;

namespace Istok::GUI::WinAPI {

struct SysMessage {
    HWND hWnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
};

using SysResult = LRESULT;

SysResult handleByDefault(SysMessage message) noexcept {
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


template <typename Window>
concept NotifierWindow = requires(Window window, MessageProxy& proxy) {
    Window(proxy);
    {window.postQueueNotification()} noexcept -> std::same_as<void>;
};

template <NotifierWindow Window>
class Notifier {
public:
    Notifier(std::shared_ptr<Window> window) : target(window) {}

    void operator()() noexcept {
        if (std::shared_ptr<Window> window = target.lock()) {
            window->postQueueNotification();
        }
    }

private:
    std::weak_ptr<Window> target;
};


template <typename WindowID, NotifierWindow Window>
class QueueProxy: public MessageProxy {
public:
    using Queue = SyncNotifyingQueue<
        GUIMessage<WindowID>,
        Notifier<Window>>;
    
    QueueProxy(GUIHandler<WindowID>& handler)
        :handler(handler) {}

    void setNotifier(std::shared_ptr<Window> window) {
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
        if (!queue) {
            return 0;
        }
        std::optional<GUIMessage<WindowID>> msg = queue->take();
        if (!msg.has_value()) {
            return 0;
        }
        handler.onMessage(msg.value());
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


template <typename Window>
concept AppWindow = requires(
    Window window,
    std::unique_ptr<WindowTranslator>&& translator
) {
    {window.setTranslator(std::move(translator))} -> std::same_as<void>;
    {window.removeTranslator()} -> std::same_as<void>;
};


template <typename WindowID, AppWindow Window>
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
        if (!storage.contains(id)) {
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


template <typename WindowID, NotifierWindow Window>
class QueueManager {
public:
    QueueManager(GUIHandler<WindowID>& handler)
        : proxy(handler),
        window(std::make_shared<Window>(proxy))
    {
        proxy.setNotifier(window);
    }

    auto getQueue() {
        return proxy.getQueue();
    }

private:
    std::shared_ptr<Window> window;
    QueueProxy<WindowID, Window> proxy;
};


template <typename Manager>
concept SysWindowManager = requires() {
    typename Manager::Window;
} && AppWindow<typename Manager::Window> && requires(
        Manager manager,
        std::shared_ptr<typename Manager::Window> window
    ) {
    {
        manager.create(std::declval<WindowParams>())
    } -> std::same_as<std::shared_ptr<typename Manager::Window>>;
    {manager.remove(window)} -> std::same_as<void>;
    {manager.runMessageLoop()} -> std::same_as<void>;
    {manager.stopMessageLoop()} noexcept -> std::same_as<void>;
};


template <typename WindowID, SysWindowManager SysManager>
class WindowManager {
public:
    using Window = SysManager::Window;
    
    WindowManager(GUIHandler<WindowID>& handler)
        : appManager(handler) {}
    
    void newWindow(WindowID id, WindowParams params) {
        std::shared_ptr<Window> window = sysManager.create(params);
        appManager.attach(id, window);
    }
    
    void destroyWindow(WindowID id) {
        std::shared_ptr<Window> window = appManager.release(id);
        sysManager.remove(window);
    }

    void runMessageLoop() {
        sysManager.runMessageLoop();
    }

    void stopMessageLoop() noexcept {
        sysManager.stopMessageLoop();
    }

private:
    SysManager sysManager;
    AppWindowManager<WindowID, Window> appManager;
};


template <
    typename WindowID_,
    NotifierWindow NWindow,
    SysWindowManager SysManager
>
class Platform {
public:
    using WindowID = WindowID_;

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
        windowManager.runMessageLoop();
    }

    void stop() noexcept {
        windowManager.stopMessageLoop();
    }

    void newWindow(WindowID id, WindowParams params) {
        windowManager.newWindow(id, params);
    }

    void destroyWindow(WindowID id) {
        windowManager.destroyWindow(id);
    }

private:
    QueueManager<WindowID, NWindow> queueManager;
    WindowManager<WindowID, SysManager> windowManager;
};

} // namespace Istok::GUI::WinAPI

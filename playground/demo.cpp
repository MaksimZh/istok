#include <ecs.hpp>
#include <gui/core/messages.hpp>
#include <gui/winapi/window.hpp>

#include <iostream>
#include <concepts>
#include <type_traits>
#include <variant>
#include <thread>
#include <queue>
#include <memory>
#include <map>


namespace CoreMsg {

struct Exit {};
struct NewWindow {};

} // namespace CoreMsg

using CoreMessage = std::variant<
    CoreMsg::Exit,
    CoreMsg::NewWindow
>;

namespace UIMsg {

struct AddMainWindow {
    Istok::ECS::Entity entity;
    std::string title;
    Position<int> position;
    Size<int> size;
};

struct AddToolWindow {
    Istok::ECS::Entity entity;
    Position<int> position;
    Size<int> size;
};

} // namespace Istok::GUI::UIMsg

using UIMessage = std::variant<
    UIMsg::AddMainWindow,
    UIMsg::AddToolWindow
>;


void threadProc(
        Istok::GUI::SyncWaitingQueue<CoreMessage>& inQueue,
        Istok::GUI::SyncNotifyingQueue<UIMessage, Notifier>& outQueue) {
    Istok::ECS::EntityComponentManager manager;
    while (true) {
        CoreMessage msg = inQueue.take();
        if (std::holds_alternative<CoreMsg::Exit>(msg)) {
            std::cout << "ecs <- Exit" << std::endl;
            break;
        }
        if (std::holds_alternative<CoreMsg::NewWindow>(msg)) {
            std::cout << "ecs <- NewWindow" << std::endl;
            outQueue.push(UIMsg::AddMainWindow(
                manager.createEntity(), "Istok", {200, 100}, {400, 300}));
        }
    }
    std::cout << "ecs end" << std::endl;
}


template<typename T>
concept UIQueue = requires(T& queue)
{
    { queue.empty() } -> std::same_as<bool>;
    { queue.take() } -> std::same_as<UIMessage>;
};


template<typename T>
concept MixedMessageHandler = requires(T& handler, UIMessage uiMsg, WinAPIMessage winMsg)
{
    { handler.handleMessage(uiMsg) } -> std::same_as<void>;
    { handler.handleMessage(winMsg) } -> std::same_as<LRESULT>;
};


template <UIQueue Queue, MixedMessageHandler Handler>
class UIMessageCollector : public WinAPIMessageHandler {
public:
    UIMessageCollector(Queue& queue, Handler& handler)
        : queue(queue), handler(handler) {}

    UIMessageCollector(const UIMessageCollector&) = delete;
    UIMessageCollector& operator=(const UIMessageCollector&) = delete;
    UIMessageCollector(UIMessageCollector&&) = delete;
    UIMessageCollector& operator=(UIMessageCollector&&) = delete;

    LRESULT handleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override {
        if (msg != WM_APP_QUEUE) {
            return handler.handleMessage(WinAPIMessage(hWnd, msg, wParam, lParam));
        }
        if (!queue.empty()) {
            handler.handleMessage(queue.take());
        }
        return 0;
    }

private:
    Queue& queue;
    Handler& handler;
};


template<typename T>
concept SysWindowMap = requires(T& map, HWND hWnd)
{
    { map.getEntity(hWnd) } -> std::same_as<Istok::ECS::Entity>;
};


template<typename T>
concept UIMessageHandler = requires(
    T& obj,
    Istok::ECS::Entity entity,
    std::string str,
    Position<int> pos,
    Size<int> size
) {
    { obj.onDestroy(entity) } -> std::same_as<void>;
    { obj.addMainWindow(entity, str, pos, size) } -> std::same_as<void>;
};


template <SysWindowMap WindowMap, UIMessageHandler Handler>
class UIMessageTranslator {
public:
    UIMessageTranslator(WindowMap& windowMap, Handler& handler)
        : windowMap(windowMap), handler(handler) {}

    UIMessageTranslator(const UIMessageTranslator&) = delete;
    UIMessageTranslator& operator=(const UIMessageTranslator&) = delete;
    UIMessageTranslator(UIMessageTranslator&&) = delete;
    UIMessageTranslator& operator=(UIMessageTranslator&&) = delete;

    void handleMessage(UIMessage message) {
        if (std::holds_alternative<UIMsg::AddMainWindow>(message)) {
            auto msg = std::get<UIMsg::AddMainWindow>(message);
            handler.addMainWindow(msg.entity, msg.title, msg.position, msg.size);
        }
    }
    
    LRESULT handleMessage(WinAPIMessage message) {
        switch (message.msg) {
        case WM_DESTROY:
            handler.onDestroy(windowMap.getEntity(message.hWnd));
            return 0;
        default:
            return DefWindowProc(
                message.hWnd,
                message.msg,
                message.wParam,
                message.lParam);
        }
    }

private:
    WindowMap& windowMap;
    Handler& handler;
};


using ECSQueue = Istok::GUI::SyncWaitingQueue<CoreMessage>;
using GUIQueue = Istok::GUI::SyncNotifyingQueue<UIMessage, Notifier>;


class WindowPool {
public:
    WindowPool() = default;
    WindowPool(const WindowPool&) = delete;
    WindowPool& operator=(const WindowPool&) = delete;
    WindowPool(WindowPool&&) = default;
    WindowPool& operator=(WindowPool&&) = default;

    void release(std::unique_ptr<SysWindow> window) {
        container.push(std::move(window));
    }

    std::unique_ptr<SysWindow> acquire() {
        if (!container.empty()) {
            return container.take();
        }
        return std::make_unique<SysWindow>();
    }

private:
    Istok::GUI::SimpleQueue<std::unique_ptr<SysWindow>> container;
};


class PreparedWindowPool {
public:
    PreparedWindowPool(const PreparedWindowPool&) = delete;
    PreparedWindowPool& operator=(const PreparedWindowPool&) = delete;
    PreparedWindowPool(PreparedWindowPool&&) = default;
    PreparedWindowPool& operator=(PreparedWindowPool&&) = default;

    PreparedWindowPool(WindowPool&& pool, WinAPIMessageHandler* handler)
        : pool(std::move(pool)), handler(handler) {}

    void release(std::unique_ptr<SysWindow> window) {
        window->hide();
        pool.release(std::move(window));
    }

    std::unique_ptr<SysWindow> acquirePrimary(
        const std::string& title,
        Position<int> position, Size<int> size
    ) {
        std::unique_ptr<SysWindow> window = pool.acquire();
        window->makePrimary(title, position, size);
        return window;
    }

    std::unique_ptr<SysWindow> acquireSecondary(
        Position<int> position, Size<int> size
    ) {
        std::unique_ptr<SysWindow> window = pool.acquire();
        window->makeSecondary(position, size);
        return window;
    }

private:
    WindowPool pool;
    WinAPIMessageHandler* handler;

    std::unique_ptr<SysWindow> acquire() {
        auto window = pool.acquire();
        window->setMessageHandler(handler);
        return window;
    }
};


class WindowMap {
public:
    WindowMap() = default;
    WindowMap(const WindowMap&) = delete;
    WindowMap& operator=(const WindowMap&) = delete;
    WindowMap(WindowMap&&) = default;
    WindowMap& operator=(WindowMap&&) = default;

    void insert(Istok::ECS::Entity entity, std::unique_ptr<SysWindow>&& window) {
        entityMap[window->getHWND()] = entity;
        windowMap[entity] = std::move(window);
    }

    Istok::ECS::Entity getEntity(HWND hWnd) {
        return entityMap.at(hWnd);
    }

    SysWindow& getSysWindow(Istok::ECS::Entity entity) {
        return *windowMap.at(entity);
    }

private:
    std::unordered_map<HWND, Istok::ECS::Entity> entityMap;
    std::unordered_map<
        Istok::ECS::Entity, std::unique_ptr<SysWindow>,
        Istok::ECS::Entity::Hasher
    > windowMap;
};


class Handler {
public:
    Handler(WindowMap& map) : map(map) {}
    
    void addMainWindow(
        Istok::ECS::Entity entity,
        const std::string& title,
        Position<int> position,
        Size<int> size
    ) {
        map.insert(entity, pool->acquirePrimary(title, position, size));
        SysWindow& sw = map.getSysWindow(entity);
        sw.show();
    }
    
    void onDestroy(Istok::ECS::Entity entity) {
        PostQuitMessage(0);
    }

private:
    WindowMap& map;
    PreparedWindowPool* pool;
};


int main() {
    WindowPool pool;
    auto sample = pool.acquire();
    Notifier notifier(sample->getHWND());
    pool.release(std::move(sample));
    ECSQueue coreQueue;
    GUIQueue uiQueue(notifier);
    WindowMap map;
    Handler handler(map);
    UIMessageTranslator translator(map, handler);
    UIMessageCollector collector(uiQueue, translator);

    std::thread ecsThread(threadProc, std::ref(coreQueue), std::ref(uiQueue));
    coreQueue.push(CoreMsg::NewWindow{});
    while (true) {
        MSG msg;
        GetMessage(&msg, NULL, 0, 0);
        if (msg.message == WM_QUIT) {
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    coreQueue.push(CoreMsg::Exit{});
    ecsThread.join();
    std::cout << "gui end" << std::endl;
    return 0;
}

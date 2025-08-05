#include <ecs.hpp>
#include <gui/core/messages.hpp>
#include <gui/winapi/window.hpp>

#include <iostream>
#include <concepts>
#include <type_traits>
#include <variant>
#include <optional>
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

struct DestroyWindow {
    Istok::ECS::Entity entity;
};

struct AddPrimaryWindow {
    Istok::ECS::Entity entity;
    std::string title;
    Position<int> position;
    Size<int> size;
};

struct AddSecondaryWindow {
    Istok::ECS::Entity entity;
    Position<int> position;
    Size<int> size;
};

} // namespace Istok::GUI::UIMsg

using UIMessage = std::variant<
    UIMsg::DestroyWindow,
    UIMsg::AddPrimaryWindow,
    UIMsg::AddSecondaryWindow
>;


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


class SysWindowMap {
public:
    SysWindowMap() = default;
    SysWindowMap(const SysWindowMap&) = delete;
    SysWindowMap& operator=(const SysWindowMap&) = delete;
    SysWindowMap(SysWindowMap&&) = default;
    SysWindowMap& operator=(SysWindowMap&&) = default;

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


class SysMessageTranslator {
public:
    SysMessageTranslator(GUIQueue& queue, SysWindowMap& windowMap)
        : queue(queue), windowMap(windowMap) {}

    SysMessageTranslator(const SysMessageTranslator&) = delete;
    SysMessageTranslator& operator=(const SysMessageTranslator&) = delete;
    SysMessageTranslator(SysMessageTranslator&&) = delete;
    SysMessageTranslator& operator=(SysMessageTranslator&&) = delete;

    std::optional<UIMessage> translate(SysMessage message) {
        switch (message.msg) {
        case WM_APP_QUEUE:
            return queue.empty() ? std::nullopt : std::optional(queue.take());
        case WM_DESTROY:
            return UIMsg::DestroyWindow(windowMap.getEntity(message.hWnd));
        default:
            return std::nullopt;
        }
    }

private:
    GUIQueue& queue;
    SysWindowMap& windowMap;
};


class Handler : SysMessageHandler {
public:
    Handler() {
        auto sample = windowPool.acquire();
        sample->setMessageHandler(this);
        coreQueue = std::make_unique<ECSQueue>();
        uiQueue = std::make_unique<GUIQueue>(Notifier(sample->getHWND()));
        windowPool.release(std::move(sample));
        messageTranslator = std::make_unique<SysMessageTranslator>(*uiQueue, windowMap);
    }

    SysResult handleSysMessage(SysMessage message) override {
        std::optional<UIMessage> optUIMessage =
            messageTranslator->translate(message);
        if (!optUIMessage.has_value()) {
            return handleSysMessageByDefault(message);
        }
        UIMessage uiMessage = optUIMessage.value();
        if (std::holds_alternative<UIMsg::DestroyWindow>(uiMessage)) {
            onDestroy(std::get<UIMsg::DestroyWindow>(uiMessage).entity);
            return 0;
        }
        if (std::holds_alternative<UIMsg::AddPrimaryWindow>(uiMessage)) {
            auto msg = std::get<UIMsg::AddPrimaryWindow>(uiMessage);
            addPrimaryWindow(msg.entity, msg.title, msg.position, msg.size);
            return 0;
        }
        return handleSysMessageByDefault(message);
    }
    
    void addPrimaryWindow(
        Istok::ECS::Entity entity,
        const std::string& title,
        Position<int> position,
        Size<int> size
    ) {
        windowMap.insert(entity, windowPool.acquire());
        SysWindow& sw = windowMap.getSysWindow(entity);
        sw.setMessageHandler(this);
        sw.makePrimary(title, position, size);
        sw.show();
    }
    
    void onDestroy(Istok::ECS::Entity entity) {
        PostQuitMessage(0);
    }

    ECSQueue& getCoreQueue() {
        return *coreQueue;
    }

    GUIQueue& getUIQueue() {
        return *uiQueue;
    }


private:
    WindowPool windowPool;
    SysWindowMap windowMap;
    std::unique_ptr<ECSQueue> coreQueue;
    std::unique_ptr<GUIQueue> uiQueue;
    std::unique_ptr<SysMessageTranslator> messageTranslator;
};


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
            outQueue.push(UIMsg::AddPrimaryWindow(
                manager.createEntity(), "Istok", {200, 100}, {400, 300}));
        }
    }
    std::cout << "ecs end" << std::endl;
}

int main() {
    Handler handler;
    ECSQueue& coreQueue = handler.getCoreQueue();
    GUIQueue& uiQueue = handler.getUIQueue();

    std::thread ecsThread(
        threadProc,
        std::ref(coreQueue),
        std::ref(uiQueue));
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

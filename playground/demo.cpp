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


class Notifier {
public:
    Notifier(SysMessageHandler* messageHandler) {
        target = std::make_unique<DCWindow>();
        target->setMessageHandler(messageHandler);
    }

    Notifier(const Notifier&) = delete;
    Notifier& operator=(const Notifier&) = delete;
    Notifier(Notifier&&) = default;
    Notifier& operator=(Notifier&&) = default;

    void operator()() {
        PostMessage(target->getHWND(), WM_APP_QUEUE, NULL, NULL);
    }

private:
    std::unique_ptr<DCWindow> target;
};


using ECSQueue = Istok::GUI::SyncWaitingQueue<CoreMessage>;
using GUIQueue = Istok::GUI::SyncNotifyingQueue<UIMessage, Notifier>;


class SysWindowMap {
public:
    SysWindowMap() = default;
    SysWindowMap(const SysWindowMap&) = delete;
    SysWindowMap& operator=(const SysWindowMap&) = delete;
    SysWindowMap(SysWindowMap&&) = default;
    SysWindowMap& operator=(SysWindowMap&&) = default;

    void insert(Istok::ECS::Entity entity, std::unique_ptr<DCWindow>&& window) {
        HWND hWnd = window->getHWND();
        assert(!entityMap.contains(hWnd));
        assert(!windowMap.contains(entity));
        entityMap[hWnd] = entity;
        windowMap[entity] = std::move(window);
    }

    bool contains(HWND hWnd) const {
        return entityMap.contains(hWnd);
    }

    bool contains(Istok::ECS::Entity entity) const {
        return windowMap.contains(entity);
    }

    Istok::ECS::Entity getEntity(HWND hWnd) {
        assert(contains(hWnd));
        return entityMap.at(hWnd);
    }

    DCWindow& getSysWindow(Istok::ECS::Entity entity) {
        assert(contains(entity));
        return *windowMap.at(entity);
    }

private:
    std::unordered_map<HWND, Istok::ECS::Entity> entityMap;
    std::unordered_map<
        Istok::ECS::Entity, std::unique_ptr<DCWindow>,
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
            if (queue.empty()) {
                return std::nullopt;
            }
            return queue.take();
        case WM_DESTROY:
            if (!windowMap.contains(message.hWnd)) {
                return std::nullopt;
            }
            return UIMsg::DestroyWindow(windowMap.getEntity(message.hWnd));
        default:
            return std::nullopt;
        }
    }

private:
    GUIQueue& queue;
    SysWindowMap& windowMap;
};


class SysMessageManager {
public:
    SysMessageManager(const SysMessageManager&) = delete;
    SysMessageManager& operator=(const SysMessageManager&) = delete;
    SysMessageManager(SysMessageManager&&) = delete;
    SysMessageManager& operator=(SysMessageManager&&) = delete;

    SysMessageManager(
        Notifier&& notifier, SysWindowMap& windowMap)
        : queue(std::move(notifier)), translator(queue, windowMap) {}
    
    std::optional<UIMessage> translate(SysMessage message) {
        return translator.translate(message);
    }

    GUIQueue& getQueue() {
        return queue;
    }

private:
    GUIQueue queue;
    SysMessageTranslator translator;
};


class SysWindowManager {
public:
    SysWindowManager(const SysWindowManager&) = delete;
    SysWindowManager& operator=(const SysWindowManager&) = delete;
    SysWindowManager(SysWindowManager&&) = delete;
    SysWindowManager& operator=(SysWindowManager&&) = delete;

    SysWindowManager(Notifier&& notifier)
        : messageManager(std::move(notifier), windowMap) {}

    std::optional<UIMessage> translate(SysMessage message) {
        return messageManager.translate(message);
    }

    GUIQueue& getQueue() {
        return messageManager.getQueue();
    }

    DCWindow& newWindow(Istok::ECS::Entity entity) {
        windowMap.insert(entity, std::move(std::make_unique<DCWindow>()));
        return windowMap.getSysWindow(entity);
    }

    bool contains(HWND hWnd) const {
        return windowMap.contains(hWnd);
    }

    bool contains(Istok::ECS::Entity entity) const {
        return windowMap.contains(entity);
    }

    Istok::ECS::Entity getEntity(HWND hWnd) {
        assert(contains(hWnd));
        return windowMap.getEntity(hWnd);
    }

    DCWindow& getSysWindow(Istok::ECS::Entity entity) {
        assert(contains(entity));
        return windowMap.getSysWindow(entity);
    }

private:
    SysWindowMap windowMap;
    SysMessageManager messageManager;
};


class GUICore {
public:
    GUICore(const GUICore&) = delete;
    GUICore& operator=(const GUICore&) = delete;
    GUICore(GUICore&&) = delete;
    GUICore& operator=(GUICore&&) = delete;

    GUICore(GUIQueue& uiQueue)
        : thread(threadProc, std::ref(queue), std::ref(uiQueue)) {}

    ~GUICore() {
        exit();
        thread.join();
    }

    void exit() {
        queue.push(CoreMsg::Exit{});
    }

    void newWindow() {
        queue.push(CoreMsg::NewWindow{});
    }

private:
    ECSQueue queue;
    std::thread thread;

    static void threadProc(ECSQueue& inQueue, GUIQueue& outQueue) {
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
};


class GUI : SysMessageHandler {
public:
    GUI() : windowManager(Notifier(this)), core(windowManager.getQueue()) {}

    void newWindow() {
        core.newWindow();
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
        core.exit();
    }

    SysResult handleSysMessage(SysMessage message) override {
        std::optional<UIMessage> optUIMessage =
            windowManager.translate(message);
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
        DCWindow& sw = windowManager.newWindow(entity);
        sw.setMessageHandler(this);
        sw.makePrimary(title, position, size);
        sw.show();
    }

    void onDestroy(Istok::ECS::Entity entity) {
        PostQuitMessage(0);
    }

private:
    SysWindowManager windowManager;
    GUICore core;
};


int main() {
    GUI handler;
    handler.newWindow();
    handler.run();
    std::cout << "gui end" << std::endl;
    return 0;
}

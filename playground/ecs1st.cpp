// ecs1st.cpp
#include <ecs.hpp>
#include <gui/core/messages.hpp>
#include <gui/winapi/window.hpp>

#include <iostream>
#include <string>
#include <thread>
#include <variant>
#include <optional>


struct GUIExit {};

struct GUINewWindow {
    Istok::ECS::Entity entity;
    WindowParams params;
};

struct GUIDestroyWindow {
    Istok::ECS::Entity entity;
};


using GUIMessage = std::variant<
    GUIExit,
    GUINewWindow,
    GUIDestroyWindow
>;


struct ECSExit {};

struct ECSWindowClosed {
    Istok::ECS::Entity entity;
};

using ECSMessage = std::variant<
    ECSExit,
    ECSWindowClosed
>;


class Notifier {
public:
    Notifier(SysWindow& window) : target(&window) {}

    Notifier(const Notifier&) = default;
    Notifier& operator=(const Notifier&) = default;
    Notifier(Notifier&&) = default;
    Notifier& operator=(Notifier&&) = default;

    void operator()() {
        target->postMessage(WM_APP_QUEUE);
    }

private:
    SysWindow* target;
};


using GUIQueue = Istok::GUI::SyncLazyNotifyingQueue<GUIMessage, Notifier>;
using ECSQueue = Istok::GUI::SyncWaitingQueue<ECSMessage>;


class SysWindowMap {
public:
    SysWindowMap() = default;
    SysWindowMap(const SysWindowMap&) = delete;
    SysWindowMap& operator=(const SysWindowMap&) = delete;
    SysWindowMap(SysWindowMap&&) = default;
    SysWindowMap& operator=(SysWindowMap&&) = default;

    void insert(Istok::ECS::Entity entity, std::unique_ptr<SysWindow>&& window) {
        SysWindow* windowKey = window.get();
        assert(!entityMap.contains(windowKey));
        assert(!windowMap.contains(entity));
        entityMap[windowKey] = entity;
        windowMap[entity] = std::move(window);
    }

    bool contains(SysWindow* window) const {
        return entityMap.contains(window);
    }

    bool contains(Istok::ECS::Entity entity) const {
        return windowMap.contains(entity);
    }

    Istok::ECS::Entity getEntity(SysWindow* window) {
        assert(contains(window));
        return entityMap.at(window);
    }

    SysWindow& getSysWindow(Istok::ECS::Entity entity) {
        assert(contains(entity));
        return *windowMap.at(entity);
    }

    void erase(Istok::ECS::Entity entity) {
        assert(contains(entity));
        SysWindow* windowKey = windowMap[entity].get();
        entityMap.erase(windowKey);
        windowMap.erase(entity);
    }

private:
    std::unordered_map<SysWindow*, Istok::ECS::Entity> entityMap;
    std::unordered_map<
        Istok::ECS::Entity, std::unique_ptr<SysWindow>,
        Istok::ECS::Entity::Hasher
    > windowMap;
};


class MessageDispatcher {
public:
    MessageDispatcher(GUIQueue& inQueue, ECSQueue& outQueue)
        : inQueue(&inQueue), outQueue(&outQueue) {}
    
    bool empty() const {
        return inQueue->empty();
    }

    GUIMessage take() {
        assert(!empty());
        return inQueue->take();
    }

    void push(ECSMessage message) {
        outQueue->push(std::move(message));
    }

private:
    GUIQueue* inQueue;
    ECSQueue* outQueue;
};


class GUIHandler : public SysMessageHandler {
public:
    GUIHandler(MessageDispatcher messageDispatcher)
        : messageDispatcher(messageDispatcher) {}

    void onClose(SysWindow& window) {
        messageDispatcher.push(ECSWindowClosed(windowMap.getEntity(&window)));
    }
    
    void onQueue() {
        while (!messageDispatcher.empty()) {
            processMessage(messageDispatcher.take());
        }
    }

    void processMessage(GUIMessage msg) {
        if (std::holds_alternative<GUIExit>(msg)) {
            PostQuitMessage(0);
            return;
        }
        if (std::holds_alternative<GUINewWindow>(msg)) {
            GUINewWindow message = std::get<GUINewWindow>(msg);
            newWindow(message.entity, message.params);
            return;
        }
        if (std::holds_alternative<GUIDestroyWindow>(msg)) {
            destroyWindow(std::get<GUIDestroyWindow>(msg).entity);
            return;
        }
    }

    void newWindow(Istok::ECS::Entity entity, WindowParams params) {
        windowMap.insert(entity, std::make_unique<SysWindow>(params, *this));
        windowMap.getSysWindow(entity).show();
    }

    void destroyWindow(Istok::ECS::Entity entity) {
        windowMap.erase(entity);
    }
    
private:
    MessageDispatcher messageDispatcher;
    SysWindowMap windowMap;
};


class GUICore {
public:
    GUICore(const GUICore&) = delete;
    GUICore& operator=(const GUICore&) = delete;
    GUICore(GUICore&&) = delete;
    GUICore& operator=(GUICore&&) = delete;

    GUICore(ECSQueue& ownerQueue)
        : thread(proc, std::ref(coreQueue), std::ref(ownerQueue)) {}

    ~GUICore() {
        coreQueue.push(GUIExit{});
        thread.join();
    }

    void newWindow(Istok::ECS::Entity entity, WindowParams params) {
        coreQueue.push(GUINewWindow(entity, params));
    }

    void destroyWindow(Istok::ECS::Entity entity) {
        coreQueue.push(GUIDestroyWindow(entity));
    }

private:
    GUIQueue coreQueue;
    std::thread thread;

    static void proc(GUIQueue& inQueue, ECSQueue& outQueue) {
        std::cout << "gui: begin" << std::endl << std::flush;
        GUIHandler handler(MessageDispatcher(inQueue, outQueue));
        SysWindow dummyWindow(WindowParams{}, handler);
        Notifier notifier(dummyWindow);
        inQueue.setNotifier(std::move(notifier));
        while (true) {
            MSG msg;
            GetMessage(&msg, NULL, 0, 0);
            if (msg.message == WM_QUIT) {
                std::cout << "gui: WM_QUIT" << std::endl << std::flush;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        outQueue.push(ECSExit{});
        std::cout << "gui: end" << std::endl << std::flush;
    }
};


class GUI {
public:
    GUI(const GUI&) = delete;
    GUI& operator=(const GUI&) = delete;
    GUI(GUI&&) = delete;
    GUI& operator=(GUI&&) = delete;
    
    GUI() : core(queue) {}

    ECSMessage getMessage() {
        return queue.take();
    }

    void newWindow(Istok::ECS::Entity entity, WindowParams params) {
        core.newWindow(entity, params);
    }

    void destroyWindow(Istok::ECS::Entity entity) {
        core.destroyWindow(entity);
    }

private:
    ECSQueue queue;
    GUICore core;
};


int main() {
    std::cout << "main: begin" << std::endl << std::flush;
    Istok::ECS::EntityComponentManager ecs;
    GUI gui;
    Istok::ECS::Entity window = ecs.createEntity();
    Istok::ECS::Entity menu = ecs.createEntity();
    gui.newWindow(window, WindowParams{{200, 100, 600, 400}, "Istok"});
    gui.newWindow(menu, WindowParams{{300, 200, 400, 500}, std::nullopt});
    while (true) {
        ECSMessage msg = gui.getMessage();
        if (std::holds_alternative<ECSExit>(msg)) {
            std::cout << "main: exit" << std::endl << std::flush;
            break;
        }
        if (std::holds_alternative<ECSWindowClosed>(msg)) {
            std::cout << "main: closed" << std::endl << std::flush;
            auto ent = std::get<ECSWindowClosed>(msg).entity;
            gui.destroyWindow(ent);
            if (ent == window) {
                break;
            }
        }
    }
    std::cout << "main: end" << std::endl << std::flush;
    return 0;
}

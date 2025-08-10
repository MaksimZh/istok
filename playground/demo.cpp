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

struct GUISetWindowColor {
    Istok::ECS::Entity entity;
    Color color;
};


using GUIMessage = std::variant<
    GUIExit,
    GUINewWindow,
    GUIDestroyWindow,
    GUISetWindowColor
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
        target->postQueueNotification();
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

    SysWindow& getWindow(Istok::ECS::Entity entity) {
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


class SysWindowManager {
public:
    SysWindowManager(const SysWindowManager&) = delete;
    SysWindowManager& operator=(const SysWindowManager&) = delete;
    SysWindowManager(SysWindowManager&&) = delete;
    SysWindowManager& operator=(SysWindowManager&&) = delete;

    SysWindowManager(std::unique_ptr<SysWindow> sample)
        : sample(std::move(sample)) {
        assert(this->sample);
    }

    void newWindow(Istok::ECS::Entity entity, WindowParams params) {
        windows.insert(
            entity,
            std::make_unique<SysWindow>(*sample, params));
    }

    void destroyWindow(Istok::ECS::Entity entity) {
        windows.erase(entity);
    }

    Istok::ECS::Entity getEntity(SysWindow* window) {
        return windows.getEntity(window);
    }

    SysWindow& getWindow(Istok::ECS::Entity entity) {
        return windows.getWindow(entity);
    }

    SysWindow& getSampleWindow() {
        return *sample;
    }

private:
    std::unique_ptr<SysWindow> sample;
    SysWindowMap windows;
};


class SysGraphicsManager {
public:
    SysGraphicsManager(const SysGraphicsManager&) = delete;
    SysGraphicsManager& operator=(const SysGraphicsManager&) = delete;
    SysGraphicsManager(SysGraphicsManager&&) = delete;
    SysGraphicsManager& operator=(SysGraphicsManager&&) = delete;

    SysGraphicsManager(SysMessageHandler& messageHandler)
        : windowManager(std::make_unique<SysWindow>(messageHandler))
        {}

    void newWindow(Istok::ECS::Entity entity, WindowParams params) {
        windowManager.newWindow(entity, params);
    }

    void destroyWindow(Istok::ECS::Entity entity) {
        windowManager.destroyWindow(entity);
    }

    Istok::ECS::Entity getEntity(SysWindow* window) {
        return windowManager.getEntity(window);
    }

    SysWindow& getWindow(Istok::ECS::Entity entity) {
        return windowManager.getWindow(entity);
    }

    SysWindow& getSampleWindow() {
        return windowManager.getSampleWindow();
    }

private:
    SysWindowManager windowManager;
};


class GUIOwnerDispatcher {
public:
    GUIOwnerDispatcher() = delete;
    GUIOwnerDispatcher(const GUIOwnerDispatcher&) = default;
    GUIOwnerDispatcher& operator=(const GUIOwnerDispatcher&) = default;
    GUIOwnerDispatcher(GUIOwnerDispatcher&&) = default;
    GUIOwnerDispatcher& operator=(GUIOwnerDispatcher&&) = default;
    
    bool empty() {
        return inQueue->empty();
    }

    GUIMessage take() {
        assert(!empty());
        return inQueue->take();
    }

    void push(ECSMessage message) {
        outQueue->push(std::move(message));
    }

    void setNotifier(Notifier&& notifier) {
        inQueue->setNotifier(std::move(notifier));
    }

private:
    GUIQueue* inQueue;
    ECSQueue* outQueue;

    GUIOwnerDispatcher(GUIQueue& inQueue, ECSQueue& outQueue)
        : inQueue(&inQueue), outQueue(&outQueue) {}
    friend class GUIDispatcher;
};


class GUIDispatcher {
public:
    bool empty() {
        return inQueue.empty();
    }

    ECSMessage take() {
        return inQueue.take();
    }

    void push(GUIMessage message) {
        outQueue.push(std::move(message));
    }

    GUIOwnerDispatcher reflect() {
        return GUIOwnerDispatcher(outQueue, inQueue);
    }

private:
    ECSQueue inQueue;
    GUIQueue outQueue;
};


class GUICore : public SysMessageHandler {
public:
    GUICore(GUIOwnerDispatcher dispatcher)
        : graphics(*this), dispatcher(dispatcher) {
        dispatcher.setNotifier(Notifier(graphics.getSampleWindow()));
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

    void onClose(SysWindow& window) {
        dispatcher.push(ECSWindowClosed(graphics.getEntity(&window)));
    }
    
    void onQueue() {
        while (!dispatcher.empty()) {
            processMessage(dispatcher.take());
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
        if (std::holds_alternative<GUISetWindowColor>(msg)) {
            GUISetWindowColor message = std::get<GUISetWindowColor>(msg);
            setWindowColor(message.entity, message.color);
            return;
        }
    }

    void newWindow(Istok::ECS::Entity entity, WindowParams params) {
        graphics.newWindow(entity, params);
        graphics.getWindow(entity).show();
    }

    void destroyWindow(Istok::ECS::Entity entity) {
        graphics.destroyWindow(entity);
    }

    void setWindowColor(Istok::ECS::Entity entity, Color color) {
        graphics.getWindow(entity).setColor(color);
    }
    
private:
    SysGraphicsManager graphics;
    GUIOwnerDispatcher dispatcher;
};


class GUI {
public:
    GUI(const GUI&) = delete;
    GUI& operator=(const GUI&) = delete;
    GUI(GUI&&) = delete;
    GUI& operator=(GUI&&) = delete;

    GUI()
        : thread(proc, dispatcher.reflect()) {}

    ~GUI() {
        dispatcher.push(GUIExit{});
        thread.join();
    }

    ECSMessage getMessage() {
        return dispatcher.take();
    }

    void newWindow(Istok::ECS::Entity entity, WindowParams params) {
        dispatcher.push(GUINewWindow(entity, params));
    }

    void destroyWindow(Istok::ECS::Entity entity) {
        dispatcher.push(GUIDestroyWindow(entity));
    }

    void setWindowColor(Istok::ECS::Entity entity, Color color) {
        dispatcher.push(GUISetWindowColor(entity, color));
    }

private:
    GUIDispatcher dispatcher;
    std::thread thread;

    static void proc(GUIOwnerDispatcher dispatcher) {
        std::cout << "gui: begin" << std::endl << std::flush;
        GUICore(dispatcher).run();
        std::cout << "gui: end" << std::endl << std::flush;
    }
};


int main() {
    std::cout << "main: begin" << std::endl << std::flush;
    Istok::ECS::EntityComponentManager ecs;
    GUI gui;
    Istok::ECS::Entity window = ecs.createEntity();
    Istok::ECS::Entity menu = ecs.createEntity();
    gui.newWindow(window, WindowParams{{200, 100, 600, 400}, "Istok"});
    gui.newWindow(menu, WindowParams{{300, 200, 400, 500}, std::nullopt});
    gui.setWindowColor(window, {240, 240, 240, 255});
    gui.setWindowColor(menu, {0, 255, 0, 150});
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

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
    SysWindowParams params;
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


class GUIHandler : public SysMessageHandler {
public:
    GUIHandler(ECSQueue& outQueue) : outQueue(outQueue) {}

    void onQueue() {
        PostQuitMessage(0);
    }
    
    void onClose(SysWindow& window) {
        outQueue.push(ECSWindowClosed{});
    }
    
private:
    ECSQueue& outQueue;
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
        thread.join();
    }

    void destroyWindow(Istok::ECS::Entity entity) {
        coreQueue.push(GUIDestroyWindow(entity));
    }

private:
    GUIQueue coreQueue;
    std::thread thread;

    static void proc(GUIQueue& inQueue, ECSQueue& outQueue) {
        std::cout << "GUICore: begin" << std::endl << std::flush;
        GUIHandler handler(outQueue);
        SysWindow dummyWindow(SysWindowParams{}, handler);
        Notifier notifier(dummyWindow);
        inQueue.setNotifier(std::move(notifier));
        SysWindow window(SysWindowParams{{200, 100, 600, 400}, "Istok"}, handler);
        window.show();
        while (true) {
            MSG msg;
            GetMessage(&msg, NULL, 0, 0);
            if (msg.message == WM_QUIT) {
                std::cout << "GUICore: WM_QUIT" << std::endl << std::flush;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        outQueue.push(ECSExit{});
        std::cout << "GUICore: end" << std::endl << std::flush;
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
    //gui.newWindow(window);
    while (true) {
        ECSMessage msg = gui.getMessage();
        if (std::holds_alternative<ECSExit>(msg)) {
            std::cout << "main: exit" << std::endl << std::flush;
            break;
        }
        if (std::holds_alternative<ECSWindowClosed>(msg)) {
            std::cout << "main: closed" << std::endl << std::flush;
            gui.destroyWindow(window);
            break;
        }
    }
    std::cout << "main: end" << std::endl << std::flush;
    return 0;
}

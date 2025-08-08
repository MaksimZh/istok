// ecs1st.cpp
#include <ecs.hpp>
#include <gui/core/messages.hpp>
#include <gui/winapi/window.hpp>

#include <iostream>
#include <string>
#include <thread>
#include <variant>
#include <optional>


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


using GUIQueue = Istok::GUI::SyncLazyNotifyingQueue<std::string, Notifier>;
using ECSQueue = Istok::GUI::SyncWaitingQueue<std::string>;


class Handler : public SysMessageHandler {
public:
    SysResult handleSysMessage(SysMessage message) override {
        if (message.msg == WM_CLOSE) {
            std::cout << "Handler: WM_CLOSE" << std::endl << std::flush;
            PostQuitMessage(0);
            return 0;
        }
        return handleSysMessageByDefault(message);
    }
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

private:
    GUIQueue coreQueue;
    std::thread thread;

    static void proc(GUIQueue& inQueue, ECSQueue& outQueue) {
        std::cout << "GUICore: begin" << std::endl << std::flush;
        Handler handler;
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
        outQueue.push("exit");
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

    std::string getMessage() {
        return queue.take();
    }

private:
    ECSQueue queue;
    GUICore core;
};


int main() {
    std::cout << "main: begin" << std::endl << std::flush;
    GUI gui;
    while (true) {
        std::string msg = gui.getMessage();
        if (msg == "exit") {
            std::cout << "main: exit" << std::endl << std::flush;
            break;
        }
    }
    std::cout << "main: end" << std::endl << std::flush;
    return 0;
}

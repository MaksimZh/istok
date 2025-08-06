// ecs1st.cpp
#include <ecs.hpp>
#include <gui/core/messages.hpp>
#include <gui/winapi/window.hpp>

#include <iostream>
#include <string>
#include <thread>
#include <variant>


class Notifier {
public:
    Notifier(HWND hWnd) : hWnd(hWnd) {}

    Notifier(const Notifier&) = delete;
    Notifier& operator=(const Notifier&) = delete;
    Notifier(Notifier&&) = default;
    Notifier& operator=(Notifier&&) = default;

    void operator()() {
        PostMessage(hWnd, WM_APP_QUEUE, NULL, NULL);
    }

private:
    HWND hWnd;
};


using GUIQueue = Istok::GUI::SyncNotifyingQueue<std::string, Notifier>;

struct InitMsg {
    GUIQueue* queue;
};

using ECSMessage = std::variant<
    std::string,
    InitMsg
>;

using ECSQueue = Istok::GUI::SyncWaitingQueue<ECSMessage>;


class Handler : public SysMessageHandler {
public:
    SysResult handleSysMessage(SysMessage message) override {
        if (message.msg == WM_DESTROY) {
            std::cout << "Handler: WM_DESTROY" << std::endl;
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

    GUICore() : thread(proc, std::ref(ownerQueue)) {}

    ~GUICore() {
        thread.join();
    }

    ECSMessage getMessage() {
        return ownerQueue.take();
    }

private:
    ECSQueue ownerQueue;
    std::thread thread;

    static void proc(ECSQueue& outQueue) {
        std::cout << "GUICore: begin" << std::endl;
        Handler handler;
        SysWindow window;
        window.setMessageHandler(&handler);
        GUIQueue inQueue(Notifier(window.getHWND()));
        outQueue.push(InitMsg(&inQueue));
        window.makePrimary("Istok", {200, 100}, {400, 300});
        window.show();
        while (true) {
            MSG msg;
            GetMessage(&msg, NULL, 0, 0);
            if (msg.message == WM_QUIT) {
                std::cout << "GUICore: WM_QUIT" << std::endl;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        outQueue.push("exit");
        std::cout << "GUICore: end" << std::endl;
    }
};


class GUI {
public:
    GUI(const GUI&) = delete;
    GUI& operator=(const GUI&) = delete;
    GUI(GUI&&) = delete;
    GUI& operator=(GUI&&) = delete;
    
    GUI() {}

    std::string getMessage() {
        ECSMessage msg = core.getMessage();
        if (!std::holds_alternative<InitMsg>(msg)) {
            return std::get<std::string>(msg);
        }
        coreQueue = std::get<InitMsg>(msg).queue;
        return getMessage();
    }

private:
    GUIQueue* coreQueue = nullptr;
    GUICore core;
};


int main() {
    std::cout << "main: begin" << std::endl;
    GUI gui;
    while (true) {
        std::string msg = gui.getMessage();
        if (msg == "exit") {
            std::cout << "main: exit" << std::endl;
            break;
        }
    }
    std::cout << "main: end" << std::endl;
    return 0;
}

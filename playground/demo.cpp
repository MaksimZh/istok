#include <ecs.hpp>
#include <gui/core/messages.hpp>
#include <gui/winapi/window.hpp>

#include <iostream>
#include <variant>
#include <thread>


namespace CoreMsg {

struct Exit {};
struct NewWindow {};

} // namespace CoreMsg

using CoreMessage = std::variant<
    CoreMsg::Exit,
    CoreMsg::NewWindow
>;

namespace UIMsg {

struct AttachWindow {
    Istok::ECS::Entity entity;
};

} // namespace Istok::GUI::UIMsg

using UIMessage = std::variant<
    UIMsg::AttachWindow
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
            outQueue.push(UIMsg::AttachWindow(manager.createEntity()));
        }
    }
    std::cout << "ecs end" << std::endl;
}


class DefaultHandler : public MessageHandler {
public:
    LRESULT handleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override {
        switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
        }
    }
};


int main() {
    DefaultHandler defaultHandler;
    DCWindow window("Istok", {200, 100}, {400, 300}, false, &defaultHandler);
    Notifier notifier(window.getHWND());
    Istok::GUI::SyncWaitingQueue<CoreMessage> ecsQueue;
    Istok::GUI::SyncNotifyingQueue<UIMessage, Notifier> guiQueue(notifier);

    std::thread ecsThread(threadProc, std::ref(ecsQueue), std::ref(guiQueue));
    ecsQueue.push(CoreMsg::NewWindow{});
    while (true) {
        MSG msg;
        GetMessage(&msg, NULL, 0, 0);
        if (msg.message == WM_QUIT) {
            break;
        }
        if (msg.message == WM_APP_QUEUE) {
            UIMessage msg = guiQueue.take();
            if (std::holds_alternative<UIMsg::AttachWindow>(msg)) {
                std::cout << "gui <- AttachWindow("
                    << std::get<UIMsg::AttachWindow>(msg).entity.value << ")"
                    << std::endl;
                window.show();
            }
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ecsQueue.push(CoreMsg::Exit{});
    ecsThread.join();
    std::cout << "gui end" << std::endl;
    return 0;
}

#include <ecs.hpp>
#include <gui/core/messages.hpp>
#include <gui/winapi/window.hpp>

#include <iostream>
#include <variant>
#include <thread>
#include <queue>
#include <memory>


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


class SysWindowManager : public WinAPIMessageHandler {
public:
    using CoreQueue = Istok::GUI::SyncWaitingQueue<CoreMessage>;
    using UIQueue = Istok::GUI::SyncNotifyingQueue<UIMessage, Notifier>;
    
    SysWindowManager() {
        std::unique_ptr<SysWindow> sample = SysWindow::newDraftWindow();
        Notifier notifier(sample->getHWND());
        coreQueue = std::make_unique<CoreQueue>();
        uiQueue = std::make_unique<UIQueue>(notifier);
        sample->setMessageHandler(this);
        sample->show();
        storage = std::move(sample);
    }

    LRESULT handleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override {
        switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_APP_QUEUE: {
            UIMessage msg = uiQueue->take();
            if (std::holds_alternative<UIMsg::AddMainWindow>(msg)) {
                std::cout << "gui <- AddWindow("
                    << std::get<UIMsg::AddMainWindow>(msg).entity.value << ")"
                    << std::endl;
            }
            return 0;
        }
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
        }
    }

    CoreQueue& getCoreQueue() {
        return *coreQueue;
    }

    UIQueue& getUIQueue() {
        return *uiQueue;
    }

private:
    std::unique_ptr<SysWindow> storage;
    std::unique_ptr<CoreQueue> coreQueue;
    std::unique_ptr<UIQueue> uiQueue;
};


int main() {
    SysWindowManager manager;

    auto& coreQueue = manager.getCoreQueue();
    auto& uiQueue = manager.getUIQueue();
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

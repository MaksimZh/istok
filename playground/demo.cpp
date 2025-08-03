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


struct WinAPIMessage {
    HWND hWnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
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
        :queue(queue), handler(handler) {}

    UIMessageCollector(const UIMessageCollector&) = delete;
    UIMessageCollector& operator=(const UIMessageCollector&) = delete;
    UIMessageCollector(UIMessageCollector&& other) = delete;
    UIMessageCollector& operator=(UIMessageCollector&& other) = delete;

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
    UIMessageTranslator(UIMessageTranslator&& other) = delete;
    UIMessageTranslator& operator=(UIMessageTranslator&& other) = delete;

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


class Map {
public:
    Istok::ECS::Entity getEntity(HWND hWnd) {
        return entity;
    }

    void put(Istok::ECS::Entity value) {
        entity = value;
    }

    void put(std::unique_ptr<SysWindow>&& value) {
        sysWindow = std::move(value);
    }

    SysWindow& getSysWindow(Istok::ECS::Entity entity) {
        return *sysWindow;
    }

private:
    Istok::ECS::Entity entity;
    std::unique_ptr<SysWindow> sysWindow;
};


class Handler {
public:
    void addMainWindow(
        Istok::ECS::Entity entity,
        const std::string& title,
        Position<int> position,
        Size<int> size
    ) {
        map.put(entity);
        SysWindow& sw = map.getSysWindow(entity);
        HWND hWnd = sw.getHWND();
        SetWindowText(hWnd, toUTF16(title).c_str());
        SetWindowLongPtr(hWnd, GWL_EXSTYLE, NULL);
        SetWindowPos(
            hWnd, HWND_TOP,
            position.x, position.y,
            size.width, size.height,
            NULL);
        sw.show();
    }
    
    void onDestroy(Istok::ECS::Entity entity) {
        if (entity != map.getEntity(0)) {
            return;
        }
        PostQuitMessage(0);
    }

    Map& getMap() {
        return map;
    }

private:
    Map map;
};

int main() {
    std::unique_ptr<SysWindow> sample = SysWindow::newDraftWindow();
    Notifier notifier(sample->getHWND());
    ECSQueue coreQueue;
    GUIQueue uiQueue(notifier);
    Handler handler;
    UIMessageTranslator translator(handler.getMap(), handler);
    UIMessageCollector collector(uiQueue, translator);
    sample->setMessageHandler(&collector);
    handler.getMap().put(std::move(sample));

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

#include "internal/entity.hpp"
#include "internal/window.hpp"
#include <cassert>
#include <logging.hpp>
#include <ecs.hpp>
#include <memory>
#include <minwindef.h>
#include <unordered_map>
#include <winapi.hpp>
#include <winbase.h>
#include <winuser.h>

using namespace Istok;

using WindowMessageHandlerFunc = std::function<LRESULT(WinAPI::WindowMessage)>;

class WindowMessageHandlerProxy : public WinAPI::WindowMessageHandler {
public:
    explicit WindowMessageHandlerProxy(WindowMessageHandlerFunc func)
    : func_(func) {}

    LRESULT handleMessage(WinAPI::WindowMessage message) noexcept override {
        return func_(message);
    }

private:
    WindowMessageHandlerFunc func_;
};

using EntityWindowMessageHandlerFunc = std::function<
    LRESULT(ECS::ECSManager&, ECS::Entity, WinAPI::WindowMessage)>;

class EntityWindowMessageDispatcher {
public:
    EntityWindowMessageDispatcher(ECS::ECSManager& ecs) : ecs_(ecs) {}

    LRESULT handleMessage(
        ECS::Entity entity, WinAPI::WindowMessage message
    ) noexcept {
        auto it = handlers_.find(message.msg);
        if (it == handlers_.end()) {
            return WinAPI::handleMessageByDefault(message);
        }
        LOG_TRACE("@{}:{}", entity.index(), WinAPI::formatMessage(message));
        return it->second(ecs_, entity, message);
    }

    void setHandler(UINT msg, EntityWindowMessageHandlerFunc func) {
        handlers_[msg] = func;
    }

private:
    CLASS_WITH_LOGGER_PREFIX("GUI.WMDispatcher", "GUI: ");
    ECS::ECSManager& ecs_;
    std::unordered_map<UINT, EntityWindowMessageHandlerFunc> handlers_;
};

enum class GUIState {
    quit,
    application,
    system
};

template <typename Component>
Component& getUnique(ECS::ECSManager& ecs) {
    assert(ecs.count<Component>() == 1);
    auto holder = *ecs.view<Component>().begin();
    return ecs.get<Component>(holder);
}

std::unique_ptr<WinAPI::WindowMessageHandler> makeWindowMessageHandler(
    ECS::ECSManager& ecs, ECS::Entity entity
) {
    using Dispatcher = std::unique_ptr<EntityWindowMessageDispatcher>;
    assert(ecs.count<Dispatcher>() == 1);
    auto* dispatcherPtr = getUnique<Dispatcher>(ecs).get();
    return std::make_unique<WindowMessageHandlerProxy>(
        [entity, dispatcherPtr](WinAPI::WindowMessage message) -> LRESULT {
        return dispatcherPtr->handleMessage(entity, message);
    });
};

void messageLoopIteration(ECS::ECSManager& ecs) {
    WITH_LOGGER_PREFIX("GUI", "GUI: ");
    if (getUnique<GUIState>(ecs) != GUIState::application) {
        LOG_TRACE("message loop iteration skipped");
        return;
    }
    MSG msg;
    GetMessage(&msg, NULL, 0, 0);
    if (msg.message == WM_QUIT) {
        LOG_DEBUG("WM_QUIT message received");
        getUnique<GUIState>(ecs) = GUIState::quit;
        return;
    }
    TranslateMessage(&msg);
    getUnique<GUIState>(ecs) = GUIState::system;
    DispatchMessage(&msg);
    getUnique<GUIState>(ecs) = GUIState::application;
}

int main() {
    SET_LOGTERM_TRACE("");
    SET_LOGOFF("WinAPI.WndProc");
    WITH_LOGGER_PREFIX("", "App: ");
    LOG_TRACE("begin");

    ECS::ECSManager ecs;

    auto master = ecs.createEntity();
    auto window = ecs.createEntity();

    auto dispatcher = std::make_unique<EntityWindowMessageDispatcher>(ecs);
    dispatcher->setHandler(
        WM_DESTROY,
        [](
            ECS::ECSManager& ecs,
            ECS::Entity entity,
            WinAPI::WindowMessage message
        ) -> LRESULT {
            assert(message.msg == WM_DESTROY);
            PostQuitMessage(0);
            return 0;
        });
    dispatcher->setHandler(
        WM_SIZE,
        [](
            ECS::ECSManager& ecs,
            ECS::Entity entity,
            WinAPI::WindowMessage message
        ) -> LRESULT {
            assert(message.msg == WM_SIZE);
            ecs.iterate();
            return 0;
        });
    ecs.insert(master, std::move(dispatcher));

    WinAPI::WndHandle wnd({100, 100, 400, 300});
    ShowWindow(wnd.getHWnd(), SW_SHOW);
    auto handler = makeWindowMessageHandler(ecs, window);
    wnd.setHandler(handler.get());
    ecs.insert(window, std::move(wnd));
    ecs.insert(window, std::move(handler));

    ecs.addLoopSystem(messageLoopIteration);
    ecs.addCleanupSystem([](ECS::ECSManager& ecs) {
        LOG_TRACE("cleanup");
    });

    ecs.insert(master, GUIState::application);
    while (getUnique<GUIState>(ecs) != GUIState::quit) {
        ecs.iterate();
    }

    LOG_TRACE("end");
    return 0;
}

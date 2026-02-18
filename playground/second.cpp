#include "internal/entity.hpp"
#include "internal/window.hpp"
#include <logging.hpp>
#include <ecs.hpp>
#include <memory>
#include <minwindef.h>
#include <unordered_map>
#include <winapi.hpp>
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

using EntityWindowMessageHandlerFunc =
    std::function<LRESULT(ECS::Entity, WinAPI::WindowMessage)>;

class EntityWindowMessageDispatcher {
public:
    LRESULT handleMessage(
        ECS::Entity entity, WinAPI::WindowMessage message
    ) noexcept {
        auto it = handlers_.find(message.msg);
        if (it == handlers_.end()) {
            return WinAPI::handleMessageByDefault(message);
        }
        return it->second(entity, message);
    }

    void setHandler(UINT msg, EntityWindowMessageHandlerFunc func) {
        handlers_[msg] = func;
    }

private:
    std::unordered_map<UINT, EntityWindowMessageHandlerFunc> handlers_;
};

int main() {
    SET_LOGGER(
        "",
        Logging::TerminalLogger::GetInstance(),
        Logging::Level::all);
    WITH_LOGGER_PREFIX("", "App: ");
    LOG_TRACE("begin");

    ECS::ECSManager ecs;
    
    EntityWindowMessageDispatcher dispatcher;
    dispatcher.setHandler(
        WM_DESTROY,
        [](ECS::Entity entity, WinAPI::WindowMessage message) -> LRESULT {
            assert(message.msg == WM_DESTROY);
            PostQuitMessage(0);
            return 0;
        });

    auto dummyEntity = ecs.createEntity();

    WinAPI::WndHandle wnd({100, 100, 400, 300});
    ShowWindow(wnd.getHWnd(), SW_SHOW);
    WindowMessageHandlerProxy handler(
        [&](WinAPI::WindowMessage message) -> LRESULT {
            return dispatcher.handleMessage(dummyEntity, message);
        });
    wnd.setHandler(&handler);

    ecs.addLoopSystem([](ECS::ECSManager& ecs) {
        LOG_TRACE("loop");
    });
    ecs.addCleanupSystem([](ECS::ECSManager& ecs) {
        LOG_TRACE("cleanup");
    });

    while (true) {
        ecs.iterate();
        MSG msg;
        GetMessage(&msg, NULL, 0, 0);
        if (msg.message == WM_QUIT) {
            LOG_TRACE("WM_QUIT message received");
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    LOG_TRACE("end");
    return 0;
}

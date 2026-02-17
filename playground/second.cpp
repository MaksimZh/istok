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

int main() {
    SET_LOGGER(
        "",
        Logging::TerminalLogger::GetInstance(),
        Logging::Level::all);
    WITH_LOGGER_PREFIX("", "App: ");
    LOG_TRACE("begin");

    WinAPI::WndHandle wnd({100, 100, 400, 300});
    ShowWindow(wnd.getHWnd(), SW_SHOW);
    WindowMessageHandlerProxy handler(
        [](WinAPI::WindowMessage message) -> LRESULT {
            if (message.msg == WM_DESTROY) {
                PostQuitMessage(0);
                return 0;
            }
            return WinAPI::handleMessageByDefault(message);
        });
    wnd.setHandler(&handler);

    ECS::ECSManager ecs;
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
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    LOG_TRACE("end");
    return 0;
}

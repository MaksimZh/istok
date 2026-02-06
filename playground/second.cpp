#include "internal/window.hpp"
#include <logging.hpp>
#include <ecs.hpp>
#include <winapi.hpp>
#include <winuser.h>

int main() {
    SET_LOGGER(
        "",
        Istok::Logging::TerminalLogger::GetInstance(),
        Istok::Logging::Level::all);
    WITH_LOGGER_PREFIX("", "App: ");
    LOG_TRACE("begin");

    Istok::WinAPI::WndHandle wnd({100, 100, 400, 300});
    ShowWindow(wnd.getHWnd(), SW_SHOW);
    
    Istok::ECS::ECSManager ecs;
    ecs.addLoopSystem([](Istok::ECS::ECSManager& ecs) {
        LOG_TRACE("loop");
    });
    ecs.addCleanupSystem([](Istok::ECS::ECSManager& ecs) {
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

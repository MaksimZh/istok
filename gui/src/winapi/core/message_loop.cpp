// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "message_loop.hpp"

#include <istok/ecs.hpp>
#include <istok/logging.hpp>

#include "winapi/base/environment.hpp"
#include "winapi/base/winapi_delegate.hpp"


namespace Istok::GUI::WinAPI {

namespace {

void messageLoopIteration(
    WinAPIDelegate& winapi,
    ECS::Entity master,
    ECS::ECSManager& ecs
) noexcept {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI.MessageLoop", "WinAPI: ");
    MSG msg = winapi.getMessage();
    if (msg.message == WM_QUIT) {
        LOG_DEBUG("WM_QUIT message received.");
        ecs.insert(master, QuitFlag{});
        return;
    }
    winapi.dispatchMessage(msg);
}

}  // namespace


bool setupMessageLoop(ECS::ECSManager& ecs) {
    return runInEnvironment(
        ecs,
        [](ECS::ECSManager& ecs, ECS::Entity master, WinAPIDelegate& winapi) {
            ecs.addLoopSystem(
                [&winapi, master](ECS::ECSManager& ecs) noexcept {
                    messageLoopIteration(winapi, master, ecs); });
            return true;
        });
}

}  // namespace Istok::GUI::WinAPI

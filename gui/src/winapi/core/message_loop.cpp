// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "message_loop.hpp"

#include <istok/ecs.hpp>
#include <istok/logging.hpp>

#include "winapi/base/delegate.hpp"
#include "winapi/base/environment.hpp"


namespace Istok::GUI::WinAPI {

namespace {

struct ProcessingMessageFlag {};

void messageLoopIteration(
    WinAPIDelegate& winapi,
    ECS::Entity master,
    ECS::ECSManager& ecs
) noexcept {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI.MessageLoop", "WinAPI: ");
    if (ecs.has<ProcessingMessageFlag>(master)) {
        LOG_TRACE("Skipping nested message loop iteration.");
        return;
    }
    ecs.insert(master, ProcessingMessageFlag{});
    MSG msg = winapi.getMessage();
    if (msg.message == WM_QUIT) {
        LOG_DEBUG("WM_QUIT message received.");
        ecs.insert(master, QuitFlag{});
        return;
    }
    winapi.dispatchMessage(msg);
    ecs.remove<ProcessingMessageFlag>(master);
}

bool setup(ECS::ECSManager& ecs, ECS::Entity master, WinAPIDelegate& winapi) {
    ecs.addBottomLoopSystem([&winapi, master](ECS::ECSManager& ecs) noexcept {
        messageLoopIteration(winapi, master, ecs); });
    return true;
}

}  // namespace


bool setupMessages(ECS::ECSManager& ecs) {
    return runInEnvironment(ecs, setup);
}

}  // namespace Istok::GUI::WinAPI

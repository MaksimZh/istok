// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "istok/winapi/message_loop.hpp"

#include <windows.h>

#include <istok/logging.hpp>

#include "istok/winapi/callback_types.hpp"


namespace Istok::WinAPI {

namespace {

struct ProcessingMessageFlag {};

void messageLoopIteration(
    ECS::ECSManager& ecs, ECS::Entity master,
    QuitCallback&& quit
) noexcept {
    WITH_LOGGER_PREFIX("GUI", "GUI: ");
    if (ecs.has<ProcessingMessageFlag>(master)) {
        LOG_TRACE("GetMessage skipped");
        return;
    }
    ecs.insert(master, ProcessingMessageFlag{});
    MSG msg;
    GetMessage(&msg, NULL, 0, 0);
    if (msg.message == WM_QUIT) {
        LOG_DEBUG("WM_QUIT message received");
        quit();
        return;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    ecs.remove<ProcessingMessageFlag>(master);
}

}  // namespace


ECS::System createMessageLoopSystem(
    ECS::Entity master, QuitCallback&& quit
) noexcept {
    return ECS::System(
        [master, q = std::move(quit)](ECS::ECSManager& ecs) mutable noexcept {
            messageLoopIteration(ecs, master, std::move(q)); });
}

}  // namespace Istok::WinAPI

// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "message_loop.hpp"

#include <windows.h>

#include <istok/logging.hpp>

#include "istok/gui/base.hpp"
#include "delegate.hpp"


namespace Istok::GUI::WinAPI {

namespace {

struct ProcessingMessageFlag {};

void messageLoopIteration(
    WinAPIDelegate& winapi,
    QuitCallback& quit,
    ECS::Entity master,
    ECS::ECSManager& ecs
) noexcept {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI.MessageLoop", "WinAPI: ");
    if (ecs.has<ProcessingMessageFlag>(master)) {
        LOG_TRACE("Skipping nested message loop iteration.");
        return;
    }
    ecs.insert(master, ProcessingMessageFlag{});
    MSG msg;
    winapi.getMessage(msg);
    if (msg.message == WM_QUIT) {
        LOG_DEBUG("WM_QUIT message received.");
        quit();
        return;
    }
    winapi.dispatchMessage(msg);
    ecs.remove<ProcessingMessageFlag>(master);
}

}  // namespace


ECS::System createMessageLoopSystem(
    WinAPIDelegate& winapi,
    QuitCallback&& quit,
    ECS::Entity master
) noexcept {
    return ECS::System(
        [&winapi, q = std::move(quit), master](
            ECS::ECSManager& ecs
        ) mutable noexcept {
            messageLoopIteration(winapi, q, master, ecs);
        });
}

}  // namespace Istok::GUI::WinAPI

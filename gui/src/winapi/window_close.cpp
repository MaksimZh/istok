// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "window_close.hpp"

#include <expected>

#include <istok/ecs.hpp>

#include "delegate.hpp"
#include "dispatcher.hpp"

namespace Istok::GUI::WinAPI {

namespace {

LRESULT closeHandler(
    WinAPIDelegate& winapi, ECS::ECSManager& ecs,
    ECS::Entity entity, WindowMessage message
) noexcept {
    assert(message.msg == WM_CLOSE);
    if (!ecs.has<EventHandlers::Close>(entity)) {
        return winapi.defWindowProc(message);
    }
    ecs.get<EventHandlers::Close>(entity).func();
    return 0;
}

}  // namespace


std::expected<void, std::string> setupWindowCloseHandling(
    WinAPIDelegate& winapi, ECS::ECSManager& ecs, ECS::Entity master
) {
    if (!ecs.isValidEntity(master)) {
        return std::unexpected("Invalid master entity.");
    }
    if (!ecs.has<std::unique_ptr<Dispatcher>>(master)) {
        return std::unexpected("No Dispatcher found on master entity.");
    }
    ecs.get<std::unique_ptr<Dispatcher>>(master)
        ->setHandler(WM_CLOSE, closeHandler);
    return {};
}

}  // namespace Istok::GUI::WinAPI

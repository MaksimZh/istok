// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "window_size.hpp"

#include <expected>

#include <istok/ecs.hpp>

#include "delegate.hpp"
#include "dispatcher.hpp"

namespace Istok::GUI::WinAPI {

namespace {

LRESULT sizeHandler(
    WinAPIDelegate& winapi, ECS::ECSManager& ecs,
    ECS::Entity entity, WindowMessage message
) noexcept {
    assert(message.msg == WM_SIZE);
    if (!ecs.has<NewWindowMarker>(entity)) {
        ecs.iterate();
    }
    return winapi.defWindowProc(message);
}

}  // namespace


std::expected<void, std::string> setupWindowSizeHandling(
    WinAPIDelegate& winapi, ECS::ECSManager& ecs, ECS::Entity master
) {
    if (!ecs.isValidEntity(master)) {
        return std::unexpected("Invalid master entity.");
    }
    if (!ecs.has<std::unique_ptr<Dispatcher>>(master)) {
        return std::unexpected("No Dispatcher found on master entity.");
    }
    ecs.get<std::unique_ptr<Dispatcher>>(master)
        ->setHandler(WM_SIZE, sizeHandler);
    return {};
}

}  // namespace Istok::GUI::WinAPI

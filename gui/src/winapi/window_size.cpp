// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "window_size.hpp"

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


bool setupWindowSizeHandling(ECS::ECSManager& ecs) {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI", "WinAPI: ");
    using WinAPIContainer = std::unique_ptr<WinAPIDelegate>;
    using DispatcherContainer = std::unique_ptr<Dispatcher>;
    if (ecs.count<WinAPIContainer>() != 1) {
        LOG_ERROR("Single WinAPIDelegate expected.");
        return false;
    }
    ECS::Entity master = *ecs.view<WinAPIContainer>().begin();
    LOG_DEBUG("Detected master entity {}", master);
    WinAPIDelegate* winapi = ecs.get<WinAPIContainer>(master).get();
    if (!winapi) {
        LOG_ERROR("Empty WinAPIDelegate found.");
        return false;
    }
    if (!ecs.has<DispatcherContainer>(master)) {
        LOG_ERROR("Dispatcher not found.");
        return false;
    }
    auto& dispatcher = ecs.get<std::unique_ptr<Dispatcher>>(master);
    if (!dispatcher) {
        LOG_ERROR("Empty Dispatcher found.");
        return false;
    }
    ecs.get<std::unique_ptr<Dispatcher>>(master)
        ->setHandler(
            WM_SIZE,
            [winapi, &ecs](
                ECS::Entity entity, const WindowMessage& message
            ) noexcept {
                return sizeHandler(*winapi, ecs, entity, message);
            });
    return true;
}

}  // namespace Istok::GUI::WinAPI

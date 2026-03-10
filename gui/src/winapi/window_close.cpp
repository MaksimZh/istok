// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "window_close.hpp"

#include <istok/ecs.hpp>

#include "delegate.hpp"
#include "dispatcher.hpp"

namespace Istok::GUI::WinAPI {

namespace {

LRESULT closeHandler(
    WinAPIDelegate& winapi, ECS::ECSManager& ecs,
    ECS::Entity entity, const WindowMessage& message
) noexcept {
    assert(message.msg == WM_CLOSE);
    if (!ecs.has<EventHandlers::Close>(entity)) {
        return winapi.defWindowProc(message);
    }
    ecs.get<EventHandlers::Close>(entity).func();
    return 0;
}

}  // namespace


bool setupWindowCloseHandling(ECS::ECSManager& ecs) {
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
            WM_CLOSE,
            [winapi, &ecs](
                ECS::Entity entity, const WindowMessage& message
            ) noexcept {
                return closeHandler(*winapi, ecs, entity, message);
            });
    return true;
}

}  // namespace Istok::GUI::WinAPI

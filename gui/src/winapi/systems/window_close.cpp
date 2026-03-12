// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "window_close.hpp"

#include <optional>

#include <istok/ecs.hpp>

#include "winapi/base/delegate.hpp"
#include "winapi/base/dispatcher.hpp"

namespace Istok::GUI::WinAPI {

namespace {

std::optional<LRESULT> closeHandler(
    ECS::ECSManager& ecs, const WindowEntityMessage& message
) noexcept {
    if (!ecs.has<EventHandlers::Close>(message.entity)) {
        return std::nullopt;
    }
    ecs.get<EventHandlers::Close>(message.entity).func();
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
            [&ecs](const WindowEntityMessage& message) noexcept {
                return closeHandler(ecs, message);
            });
    return true;
}

}  // namespace Istok::GUI::WinAPI

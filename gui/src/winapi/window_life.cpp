// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "window_life.hpp"

#include <istok/ecs.hpp>

#include "base/delegate.hpp"
#include "base/dispatcher.hpp"
#include "base/window.hpp"

namespace Istok::GUI::WinAPI {

namespace {

WindowMessageHandler makeWindowMessageHandler(
    Dispatcher& dispatcher, ECS::Entity entity
) noexcept {
    return [&dispatcher, entity](const WindowMessage& message) noexcept {
        return dispatcher.handleMessage(entity, message);
    };
}

void createWindows(
    WinAPIDelegate& winapi, Dispatcher& dispatcher, ECS::ECSManager& ecs
) noexcept {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI", "WinAPI: ");
    ecs.removeAll<NewWindowMarker>();
    for (auto entity : ecs.view<CreateWindowMarker, WindowLocation>()) {
        LOG_DEBUG("Creating window {}", entity);
        Window window(
            winapi, ecs.get<WindowLocation>(entity).rect,
            makeWindowMessageHandler(dispatcher, entity));
        ecs.insert(entity, std::move(window));
        ecs.insert(entity, NewWindowMarker{});
    }
    ecs.removeAll<CreateWindowMarker>();
}

ECS::System makeCreateWindowsSystem(
    WinAPIDelegate& winapi, Dispatcher& dispatcher
) {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI", "WinAPI: ");
    return ECS::System{[&winapi, &dispatcher](ECS::ECSManager& ecs) noexcept {
        createWindows(winapi, dispatcher, ecs); }};
}

void destroyWindows(ECS::ECSManager& ecs) noexcept {
    ecs.removeAll<Window>();
}

}  // namespace


bool setupWindowLife(ECS::ECSManager& ecs) {
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
    ecs.addLoopSystem(makeCreateWindowsSystem(*winapi, *dispatcher));
    ecs.addCleanupSystem(destroyWindows);
    return true;
}

}  // namespace Istok::GUI::WinAPI

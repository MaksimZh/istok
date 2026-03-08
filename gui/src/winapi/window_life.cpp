// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "window_life.hpp"

#include <expected>

#include <istok/ecs.hpp>

#include "delegate.hpp"
#include "dispatcher.hpp"
#include "window.hpp"

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


std::expected<void, std::string> setupWindowLife(
    WinAPIDelegate& winapi, ECS::ECSManager& ecs, ECS::Entity master
) {
    if (!ecs.isValidEntity(master)) {
        return std::unexpected("Invalid master entity.");
    }
    if (!ecs.has<std::unique_ptr<Dispatcher>>(master)) {
        return std::unexpected("No Dispatcher found on master entity.");
    }
    auto& dispatcher = ecs.get<std::unique_ptr<Dispatcher>>(master);
    if (!dispatcher) {
        return std::unexpected("Empty Dispatcher found on master entity.");
    }
    ecs.addLoopSystem(makeCreateWindowsSystem(winapi, *dispatcher));
    ecs.addCleanupSystem(destroyWindows);
    return {};
}

}  // namespace Istok::GUI::WinAPI

// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "window_life.hpp"

#include <istok/ecs.hpp>
#include <istok/logging.hpp>

#include "winapi/base/delegate.hpp"
#include "winapi/base/dispatcher.hpp"
#include "winapi/base/tools.hpp"
#include "winapi/base/window.hpp"

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
    return ECS::System{[&winapi, &dispatcher](ECS::ECSManager& ecs) noexcept {
        createWindows(winapi, dispatcher, ecs); }};
}

void destroyWindows(ECS::ECSManager& ecs) noexcept {
    ecs.removeAll<Window>();
}

bool setup(
    ECS::ECSManager& ecs, ECS::Entity master, WinAPIDelegate& winapi
) {
    auto dispatcherContainer = std::make_unique<Dispatcher>(winapi);
    Dispatcher& dispatcher = *dispatcherContainer;
    ecs.insert(master, std::move(dispatcherContainer));
    ecs.addLoopSystem(makeCreateWindowsSystem(winapi, dispatcher));
    ecs.addCleanupSystem(destroyWindows);
    return true;
}

}  // namespace


bool setupWindowLife(ECS::ECSManager& ecs) {
    return runInEnvironment(ecs, setup);
}

}  // namespace Istok::GUI::WinAPI

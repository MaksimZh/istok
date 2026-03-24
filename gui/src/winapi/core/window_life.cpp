// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "window_life.hpp"

#include <istok/ecs.hpp>
#include <istok/logging.hpp>

#include "winapi/base/dispatcher.hpp"
#include "winapi/base/environment.hpp"
#include "winapi/base/winapi_delegate.hpp"
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
    ECS::ECSManager& ecs, WinAPIDelegate& winapi, Dispatcher& dispatcher
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

}  // namespace


bool setupWindowLife(ECS::ECSManager& ecs) {
    return runInEnvironment(
        ecs,
        [](
            ECS::ECSManager& ecs, ECS::Entity master, WinAPIDelegate& winapi
        ) {
            auto dispatcherContainer = std::make_unique<Dispatcher>(winapi);
            Dispatcher& dispatcher = *dispatcherContainer;
            ecs.insert(master, std::move(dispatcherContainer));
            ecs.addLoopSystem([&ecs, &winapi, &dispatcher]() noexcept {
                createWindows(ecs, winapi, dispatcher); });
            ecs.addTailCleanupSystem([&ecs]() noexcept {
                ecs.removeAll<Window>();
            });
            return true;
        });
}

}  // namespace Istok::GUI::WinAPI

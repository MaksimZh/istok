// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "setup.hpp"

#include <memory>

#include <istok/logging.hpp>

#include "istok/gui/base.hpp"

#include "dispatcher_setup.hpp"
#include "message_loop.hpp"
#include "real_winapi.hpp"
#include "window.hpp"

namespace Istok::GUI::WinAPI {

namespace {

void createWindows(
    WinAPIDelegate& winapi, WindowMessageHandlerGenerator& handlerGenerator,
    ECS::ECSManager& ecs
) noexcept {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI", "WinAPI: ");
    ecs.removeAll<NewWindowMarker>();
    for (auto entity : ecs.view<CreateWindowMarker, WindowLocation>()) {
        LOG_DEBUG("Creating window {}", entity);
        Window window(winapi, ecs.get<WindowLocation>(entity).rect);
        window.setMessageHandler(handlerGenerator(entity));
        ecs.insert(entity, std::move(window));
        ecs.insert(entity, NewWindowMarker{});
    }
    ecs.removeAll<CreateWindowMarker>();
}

ECS::System createCreateWindowSystem(
    WinAPIDelegate& winapi, WindowMessageHandlerGenerator&& handlerGenerator
) {
    return ECS::System{
        [&winapi, hg = std::move(handlerGenerator)](
            ECS::ECSManager& ecs
        ) mutable noexcept {
            createWindows(winapi, hg, ecs);
        }
    };
}

void destroyWindows(ECS::ECSManager& ecs) noexcept {
    ecs.removeAll<Window>();
}


void showWindows(WinAPIDelegate& winapi, ECS::ECSManager& ecs) noexcept {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI", "WinAPI: ");
    for (auto entity : ecs.view<ShowWindowMarker, Window>()) {
        LOG_DEBUG("Showing window {}", entity);
        HWND hWnd = ecs.get<Window>(entity).getHWnd();
        winapi.showWindow(hWnd);
    }
    ecs.removeAll<ShowWindowMarker>();
}

ECS::System createShowWindowSystem(WinAPIDelegate& winapi) {
    return ECS::System{
        [&winapi](ECS::ECSManager& ecs) noexcept {
            showWindows(winapi, ecs);
        }
    };
}

}  // namespace


void setupGUIWinAPI(ECS::ECSManager& ecs, QuitCallback&& quit) {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI", "WinAPI: ");
    LOG_TRACE("Start GUI system setup.");

    auto master = ecs.createEntity();
    LOG_DEBUG("Created master entity {}.", master);

    auto winapiContainer = std::make_unique<RealWinAPI>();
    WinAPIDelegate& winapi = *winapiContainer;
    ecs.insert(master, std::move(winapiContainer));

    WindowMessageHandlerGenerator handlerGenerator =
        setupDispatcher(winapi, ecs, master);
    ecs.addLoopSystem(
        createCreateWindowSystem(winapi, std::move(handlerGenerator)));
    ecs.addCleanupSystem(destroyWindows);
    ecs.addLoopSystem(createShowWindowSystem(winapi));
    ecs.addBottomLoopSystem(
        createMessageLoopSystem(winapi, std::move(quit), master));
}

}  // namespace Istok::GUI::WinAPI

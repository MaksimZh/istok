// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "setup.hpp"

#include <memory>

#include <istok/logging.hpp>

#include "istok/gui/base.hpp"

#include "dispatcher.hpp"
#include "message_loop.hpp"
#include "real_winapi.hpp"
#include "window.hpp"
#include "window_close.hpp"
#include "window_size.hpp"

namespace Istok::GUI::WinAPI {

namespace {

void createWindows(
    WinAPIDelegate& winapi, ECS::Entity master,
    ECS::ECSManager& ecs
) noexcept {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI", "WinAPI: ");
    ecs.removeAll<NewWindowMarker>();
    if (!ecs.has<std::unique_ptr<Dispatcher>>(master)) {
        LOG_ERROR("No Dispatcher on {}", master);
        return;
    }
    Dispatcher* dispatcherPtr =
        ecs.get<std::unique_ptr<Dispatcher>>(master).get();
    for (auto entity : ecs.view<CreateWindowMarker, WindowLocation>()) {
        LOG_DEBUG("Creating window {}", entity);
        Window window(winapi, ecs.get<WindowLocation>(entity).rect);
        window.setMessageHandler(
            [entity, dispatcherPtr](const WindowMessage& message) noexcept {
                return dispatcherPtr->handleMessage(entity, message);
            });
        ecs.insert(entity, std::move(window));
        ecs.insert(entity, NewWindowMarker{});
    }
    ecs.removeAll<CreateWindowMarker>();
}

ECS::System createCreateWindowSystem(
    WinAPIDelegate& winapi, ECS::Entity master
) {
    return ECS::System{[&winapi, master](ECS::ECSManager& ecs) noexcept {
        createWindows(winapi, master, ecs); }};
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

    ecs.insert(master, std::make_unique<Dispatcher>(winapi, ecs));
    if (auto result = setupWindowCloseHandling(winapi, ecs, master); !result) {
        LOG_ERROR("Setup window close: {}", result.error());
        return;
    }
    if (auto result = setupWindowSizeHandling(winapi, ecs, master); !result) {
        LOG_ERROR("Setup window size: {}", result.error());
        return;
    }
    ecs.addLoopSystem(createCreateWindowSystem(winapi, master));
    ecs.addCleanupSystem(destroyWindows);
    ecs.addLoopSystem(createShowWindowSystem(winapi));
    ecs.addBottomLoopSystem(
        createMessageLoopSystem(winapi, std::move(quit), master));
}

}  // namespace Istok::GUI::WinAPI

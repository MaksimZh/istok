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
#include "window_life.hpp"
#include "window_size.hpp"

namespace Istok::GUI::WinAPI {

namespace {

void showWindows(WinAPIDelegate& winapi, ECS::ECSManager& ecs) noexcept {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI", "WinAPI: ");
    for (auto entity : ecs.view<ShowWindowMarker, Window>()) {
        LOG_DEBUG("Showing window {}", entity);
        HWND hWnd = ecs.get<Window>(entity).getHWnd();
        winapi.showWindow(hWnd);
    }
    ecs.removeAll<ShowWindowMarker>();
}

ECS::System makeShowWindowsSystem(WinAPIDelegate& winapi) {
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

    ecs.insert(master, std::make_unique<RealWinAPI>());
    WinAPIDelegate& winapi = *ecs.get<std::unique_ptr<RealWinAPI>>(master);
    ecs.insert(master, std::make_unique<Dispatcher>(winapi, ecs));

    if (auto result = setupWindowLife(winapi, ecs, master); !result) {
        LOG_ERROR("Setup window life: {}", result.error());
        return;
    }
    if (auto result = setupWindowCloseHandling(winapi, ecs, master); !result) {
        LOG_ERROR("Setup window close: {}", result.error());
        return;
    }
    if (auto result = setupWindowSizeHandling(winapi, ecs, master); !result) {
        LOG_ERROR("Setup window size: {}", result.error());
        return;
    }
    ecs.addLoopSystem(makeShowWindowsSystem(winapi));
    ecs.addBottomLoopSystem(
        createMessageLoopSystem(winapi, std::move(quit), master));
}

}  // namespace Istok::GUI::WinAPI

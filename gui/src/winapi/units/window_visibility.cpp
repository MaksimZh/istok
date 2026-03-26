// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "window_visibility.hpp"

#include <istok/ecs.hpp>
#include <istok/gui.hpp>
#include <istok/logging.hpp>

#include "winapi/base/environment.hpp"
#include "winapi/base/winapi_delegate.hpp"
#include "winapi/base/window.hpp"

namespace Istok::GUI::WinAPI {

namespace {

void showWindows(ECS::ECSManager& ecs, WinAPIDelegate& winapi) noexcept {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI", "WinAPI: ");
    for (auto entity : ecs.view<ShowWindowMarker, Window>()) {
        LOG_DEBUG("Showing window {}", entity);
        HWND hWnd = ecs.get<Window>(entity).getHWnd();
        winapi.showWindow(hWnd);
    }
    ecs.removeAll<ShowWindowMarker>();
}

}  // namespace


bool setupWindowVisibility(ECS::ECSManager& ecs) {
    return runInEnvironment(
        ecs,
        [](ECS::ECSManager& ecs, WinAPIDelegate& winapi) {
            ecs.addLoopSystem(
                [&ecs, &winapi]() noexcept {showWindows(ecs, winapi); });
            return true;
        });
}

}  // namespace Istok::GUI::WinAPI

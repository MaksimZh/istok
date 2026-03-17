// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "window_visibility.hpp"

#include <istok/ecs.hpp>
#include <istok/gui.hpp>
#include <istok/logging.hpp>

#include "istok/gui/base.hpp"
#include "winapi/base/winapi_delegate.hpp"
#include "winapi/base/window.hpp"

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
    return ECS::System{[&winapi](ECS::ECSManager& ecs) noexcept {
        showWindows(winapi, ecs); }};
}

}  // namespace

bool setupWindowVisibility(ECS::ECSManager& ecs) {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI", "WinAPI: ");
    using WinAPIContainer = std::unique_ptr<WinAPIDelegate>;
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
    ecs.addLoopSystem(makeShowWindowsSystem(*winapi));
    return true;
}

}  // namespace Istok::GUI::WinAPI

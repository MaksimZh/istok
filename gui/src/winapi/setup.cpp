// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "setup.hpp"

#include <memory>

#include <istok/logging.hpp>

#include "istok/gui/base.hpp"

#include "base/dispatcher.hpp"
#include "base/message_loop.hpp"
#include "real_winapi.hpp"
#include "systems/window_close.hpp"
#include "systems/window_life.hpp"
#include "systems/window_size.hpp"
#include "systems/window_visibility.hpp"

namespace Istok::GUI::WinAPI {

void setupGUIWinAPI(ECS::ECSManager& ecs, QuitCallback&& quit) {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI", "WinAPI: ");
    LOG_TRACE("Start GUI system setup.");

    auto master = ecs.createEntity();
    LOG_DEBUG("Created master entity {}.", master);

    auto winapiContainer = std::make_unique<RealWinAPI>();
    WinAPIDelegate& winapi = *winapiContainer;
    ecs.insert(
        master, std::unique_ptr<WinAPIDelegate>{std::move(winapiContainer)});
    ecs.insert(master, std::make_unique<Dispatcher>(winapi));

    if (!setupWindowLife(ecs)) {
        return;
    }
    if (!setupWindowCloseHandling(ecs)) {
        return;
    }
    if (!setupWindowSizeHandling(ecs)) {
        return;
    }
    if (!setupWindowVisibilityHandling(ecs)) {
        return;
    }
    ecs.addBottomLoopSystem(
        createMessageLoopSystem(winapi, std::move(quit), master));
    LOG_DEBUG("Setup succeeded.");
}

}  // namespace Istok::GUI::WinAPI

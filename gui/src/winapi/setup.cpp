// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "setup.hpp"

#include <memory>
#include <vector>

#include <istok/logging.hpp>

#include "istok/gui/base.hpp"

#include "base/dispatcher.hpp"
#include "base/message_loop.hpp"
#include "real_winapi.hpp"
#include "units/window_close.hpp"
#include "units/window_life.hpp"
#include "units/window_size.hpp"
#include "units/window_visibility.hpp"

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

    const std::vector<bool(*)(ECS::ECSManager&)> units = {
        setupWindowLife,
        setupWindowClose,
        setupWindowSize,
        setupWindowVisibility,
    };
    for (const auto& unit : units) {
        if (!unit(ecs)) {
            return;
        }
    }
    ecs.addBottomLoopSystem(
        createMessageLoopSystem(winapi, std::move(quit), master));
    LOG_DEBUG("Setup succeeded.");
}

}  // namespace Istok::GUI::WinAPI

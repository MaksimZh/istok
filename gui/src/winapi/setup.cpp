// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "setup.hpp"

#include <memory>
#include <vector>

#include <istok/logging.hpp>

#include "winapi/real_winapi.hpp"
#include "winapi/core/message_loop.hpp"
#include "winapi/core/window_life.hpp"
#include "winapi/units/window_close.hpp"
#include "winapi/units/window_size.hpp"
#include "winapi/units/window_visibility.hpp"

namespace Istok::GUI::WinAPI {

bool setupGUIWinAPI(ECS::ECSManager& ecs) {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI", "WinAPI: GUI setup: ");

    auto master = ecs.createEntity();
    LOG_DEBUG("Created master entity {}.", master);
    ecs.insert(master, std::unique_ptr<WinAPIDelegate>{
        std::make_unique<RealWinAPI>()});

    const std::vector<bool(*)(ECS::ECSManager&)> units = {
        setupWindowLife,
        setupWindowClose,
        setupWindowSize,
        setupWindowVisibility,
        setupMessageLoop,
    };
    for (const auto& unit : units) {
        if (!unit(ecs)) {
            LOG_DEBUG("Failed.");
            return false;
        }
    }
    LOG_DEBUG("Succeeded.");
    return true;
}

}  // namespace Istok::GUI::WinAPI

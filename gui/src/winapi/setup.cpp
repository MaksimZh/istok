// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "setup.hpp"

#include <memory>
#include <vector>

#include <istok/logging.hpp>

#include "real_winapi.hpp"
#include "units/messages.hpp"
#include "units/window_close.hpp"
#include "units/window_life.hpp"
#include "units/window_size.hpp"
#include "units/window_visibility.hpp"

namespace Istok::GUI::WinAPI {

void setupGUIWinAPI(ECS::ECSManager& ecs) {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI", "WinAPI: GUI setup: ");

    auto master = ecs.createEntity();
    LOG_DEBUG("Created master entity {}.", master);
    ecs.insert(master, std::unique_ptr<WinAPIDelegate>{
        std::make_unique<RealWinAPI>()});

    const std::vector<bool(*)(ECS::ECSManager&)> units = {
        setupMessages,
        setupWindowLife,
        setupWindowClose,
        setupWindowSize,
        setupWindowVisibility,
    };
    for (const auto& unit : units) {
        if (!unit(ecs)) {
            LOG_DEBUG("Failed.");
            return;
        }
    }
    LOG_DEBUG("Succeeded.");
}

}  // namespace Istok::GUI::WinAPI

// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "setup.hpp"

#include <vector>

#include <istok/logging.hpp>

#include "winapi/core/messages.hpp"
#include "winapi/core/window_life.hpp"

namespace Istok::GUI::WinAPI {

bool setupWinAPICore(ECS::ECSManager& ecs) {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI", "WinAPI: ");

    const std::vector<bool(*)(ECS::ECSManager&)> units = {
        setupMessages,
        setupWindowLife,
    };
    for (const auto& unit : units) {
        if (!unit(ecs)) {
            LOG_DEBUG("WinAPI Core setup failed.");
            return false;
        }
    }
    LOG_DEBUG("WinAPI Core setup succeeded.");
    return true;
}

}  // namespace Istok::GUI::WinAPI

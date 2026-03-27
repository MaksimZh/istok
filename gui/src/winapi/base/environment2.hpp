// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <istok/ecs.hpp>
#include "winapi/base/dispatcher2.hpp"


// TODO:
// Add WindowManager interface.
// Make it master marker instead of Dispatcher2.
// Move all low-level dispatcher handler setup to WindowManager as they are
//  not plugins but rather parts of monolith logic (extendible though).
// Add FakeWindowManager that creates no real windows.
// Add painting processing and Renderer as injected dependency.

namespace Istok::GUI::WinAPI {

bool runInEnvironment(
    ECS::ECSManager& ecs,
    std::move_only_function<bool(
        ECS::ECSManager&,
        ECS::Entity,
        Dispatcher2&
    )> func);

bool runInEnvironment(
    ECS::ECSManager& ecs,
    std::move_only_function<bool(
        ECS::ECSManager&,
        Dispatcher2&
    )> func);

}  // namespace Istok::GUI::WinAPI
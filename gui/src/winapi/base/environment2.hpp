// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <istok/ecs.hpp>
#include "winapi/base/dispatcher2.hpp"


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
// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <istok/ecs.hpp>
#include "winapi/base/delegate.hpp"
#include "winapi/base/dispatcher.hpp"


namespace Istok::GUI::WinAPI {

bool runInEnvironment(
    ECS::ECSManager& ecs,
    std::move_only_function<bool(
        ECS::ECSManager&,
        ECS::Entity,
        WinAPIDelegate&
    )> func);

bool runInEnvironment(
    ECS::ECSManager& ecs,
    std::move_only_function<bool(
        ECS::ECSManager&,
        WinAPIDelegate&
    )> func);

bool runInEnvironment(
    ECS::ECSManager& ecs,
    std::move_only_function<bool(
        ECS::ECSManager&,
        ECS::Entity,
        WinAPIDelegate&,
        Dispatcher&
    )> func);

bool runInEnvironment(
    ECS::ECSManager& ecs,
    std::move_only_function<bool(
        ECS::ECSManager&,
        WinAPIDelegate&,
        Dispatcher&
    )> func);

}  // namespace Istok::GUI::WinAPI

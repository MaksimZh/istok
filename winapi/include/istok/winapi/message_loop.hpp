// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <istok/ecs.hpp>

#include "istok/winapi/callback_types.hpp"

namespace Istok::WinAPI {

ECS::System createMessageLoopSystem(
    ECS::Entity master, QuitCallback&& quit) noexcept;

}  // namespace Istok::WinAPI

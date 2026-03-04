// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <functional>

#include <istok/ecs.hpp>

namespace Istok::WinAPI {

using QuitCallback = std::move_only_function<void() noexcept>;
ECS::System createMessageLoopSystem(
    ECS::Entity master, QuitCallback&& quit) noexcept;

}  // namespace Istok::WinAPI

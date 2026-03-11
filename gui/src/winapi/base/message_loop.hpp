// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <istok/ecs.hpp>

#include "istok/gui/base.hpp"
#include "winapi/base/delegate.hpp"

namespace Istok::GUI::WinAPI {

ECS::System createMessageLoopSystem(
    WinAPIDelegate& winapi, QuitCallback&& quit, ECS::Entity master) noexcept;

}  // namespace Istok::GUI::WinAPI

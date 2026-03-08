// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <expected>

#include <istok/ecs.hpp>

#include "delegate.hpp"

namespace Istok::GUI::WinAPI {

std::expected<void, std::string> setupWindowLife(
    WinAPIDelegate& winapi, ECS::ECSManager& ecs, ECS::Entity master);

}  // namespace Istok::GUI::WinAPI

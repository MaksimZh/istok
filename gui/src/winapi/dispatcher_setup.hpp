// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <istok/ecs.hpp>

#include "delegate.hpp"

namespace Istok::GUI::WinAPI {

void setupDispatcher(
    WinAPIDelegate& winapi, ECS::ECSManager& ecs, ECS::Entity master);

}  // namespace Istok::GUI::WinAPI

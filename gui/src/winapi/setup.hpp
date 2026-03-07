// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <istok/ecs.hpp>

#include "istok/gui/base.hpp"

namespace Istok::GUI::WinAPI {

void setupGUIWinAPI(ECS::ECSManager& ecs, QuitCallback&& quit);

}  // namespace Istok::GUI::WinAPI

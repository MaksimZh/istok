// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <istok/ecs.hpp>

#include "gui/base.hpp"

namespace Istok::GUI {

void setupGUI(ECS::ECSManager& ecs, QuitCallback&& quit);

}  // namespace Istok::GUI

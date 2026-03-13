// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved

#include "istok/gui.hpp"

#ifdef _WIN32
#include "winapi/setup.hpp"
#endif

namespace Istok::GUI {

#ifdef _WIN32
void setupGUI(ECS::ECSManager& ecs) {
    WinAPI::setupGUIWinAPI(ecs);
}
#endif

}  // namespace Istok::GUI

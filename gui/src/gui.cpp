// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved

#include "istok/gui.hpp"

#ifdef _WIN32
#include "winapi/setup.hpp"
#endif

namespace Istok::GUI {

#ifdef _WIN32
bool setupGUI(ECS::ECSManager& ecs) {
    return WinAPI::setupGUIWinAPI(ecs);
}
#endif

}  // namespace Istok::GUI

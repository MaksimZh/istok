// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once
#include <gui/common/core.hpp>
#include <gui/winapi/platform.hpp>
#include <gui/winapi/window.hpp>
#include <ecs.hpp>


namespace Istok::GUI {

using GUIManager = GUIFor<WinAPI::Platform<
    Istok::ECS::Entity,
    WinAPI::WinAPINotifierWindow,
    WinAPI::WinAPIWindowManager>>;

} // namespace Istok::GUI

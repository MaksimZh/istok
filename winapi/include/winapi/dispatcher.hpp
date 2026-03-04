// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <functional>

#include <windows.h>

#include <logging.hpp>
#include <ecs.hpp>

#include <winapi/window.hpp>


namespace Istok::WinAPI {

class WindowMessageDispatcher {
public:
    using Handler = std::function<
        LRESULT(ECS::ECSManager&, ECS::Entity, WinAPI::WindowMessage)>;

    WindowMessageDispatcher(ECS::ECSManager& ecs) : ecs_(ecs) {}

    LRESULT handleMessage(
        ECS::Entity entity, WinAPI::WindowMessage message
    ) noexcept;

    void setHandler(UINT msg, Handler func);

private:
    CLASS_WITH_LOGGER_PREFIX("GUI.WMDispatcher", "GUI: ");
    ECS::ECSManager& ecs_;
    std::unordered_map<UINT, Handler> handlers_;
};

}  // namespace Istok::WinAPI

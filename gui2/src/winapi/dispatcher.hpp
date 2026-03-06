// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <functional>

#include <windows.h>

#include <istok/ecs.hpp>
#include <istok/logging.hpp>

#include "delegate.hpp"
#include "message.hpp"


namespace Istok::GUI::WinAPI {

class WindowMessageDispatcher {
public:
    using Handler = std::move_only_function<LRESULT(
        ECS::ECSManager&, ECS::Entity, WindowMessage) noexcept>;

    WindowMessageDispatcher(WinAPIDelegate& winapi, ECS::ECSManager& ecs)
    : winapi_(winapi), ecs_(ecs) {}

    WindowMessageDispatcher(const WindowMessageDispatcher&) = delete;
    WindowMessageDispatcher& operator=(const WindowMessageDispatcher&) = delete;
    WindowMessageDispatcher(WindowMessageDispatcher&&) = delete;
    WindowMessageDispatcher& operator=(WindowMessageDispatcher&&) = delete;

    LRESULT handleMessage(
        ECS::Entity entity, const WindowMessage& message) noexcept;
    void setHandler(UINT msg, Handler&& handler);

private:
    CLASS_WITH_LOGGER_PREFIX("WinAPI.Dispatcher", "WinAPI: ");
    WinAPIDelegate& winapi_;
    ECS::ECSManager& ecs_;
    std::unordered_map<UINT, Handler> handlers_;
};

}  // namespace Istok::WinAPI

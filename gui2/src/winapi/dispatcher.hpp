// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <functional>

#include <windows.h>

#include <istok/ecs.hpp>
#include <istok/logging.hpp>

#include "delegate.hpp"
#include "message.hpp"


namespace Istok::GUI::WinAPI {

class Dispatcher {
public:
    using Handler = std::move_only_function<LRESULT(
        ECS::ECSManager&, ECS::Entity, WindowMessage) noexcept>;

    Dispatcher(WinAPIDelegate& winapi, ECS::ECSManager& ecs)
    : winapi_(winapi), ecs_(ecs) {}

    Dispatcher(const Dispatcher&) = delete;
    Dispatcher& operator=(const Dispatcher&) = delete;
    Dispatcher(Dispatcher&&) = delete;
    Dispatcher& operator=(Dispatcher&&) = delete;

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

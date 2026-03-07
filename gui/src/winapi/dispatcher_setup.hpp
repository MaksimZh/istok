// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <functional>

#include <istok/ecs.hpp>

#include "delegate.hpp"
#include "message.hpp"

namespace Istok::GUI::WinAPI {

using WindowMessageHandlerGenerator = std::move_only_function<
    WindowMessageHandler(ECS::Entity) noexcept>;

WindowMessageHandlerGenerator setupDispatcher(
    WinAPIDelegate& winapi, ECS::ECSManager& ecs, ECS::Entity master);

}  // namespace Istok::GUI::WinAPI

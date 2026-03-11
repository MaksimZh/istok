// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <functional>
#include <optional>

#include <windows.h>

#include <istok/ecs.hpp>
#include <istok/logging.hpp>

#include "winapi/base/delegate.hpp"
#include "winapi/base/message.hpp"


namespace Istok::GUI::WinAPI {

class Dispatcher {
public:
    using Handler = std::move_only_function<
        std::optional<LRESULT>(ECS::Entity, const WindowMessage&) noexcept>;

    Dispatcher(WinAPIDelegate& winapi) : winapi_(winapi) {}

    Dispatcher(const Dispatcher&) = delete;
    Dispatcher& operator=(const Dispatcher&) = delete;
    Dispatcher(Dispatcher&&) = delete;
    Dispatcher& operator=(Dispatcher&&) = delete;

    LRESULT handleMessage(
        ECS::Entity entity, const WindowMessage& message) noexcept;
    void setHandler(UINT msg, Handler&& handler) noexcept;

private:
    CLASS_WITH_LOGGER_PREFIX("Istok.GUI.WinAPI.Dispatcher", "WinAPI: ");
    WinAPIDelegate& winapi_;
    std::unordered_map<UINT, Handler> handlers_;
};

}  // namespace Istok::WinAPI

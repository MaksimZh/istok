// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <functional>

#include <windows.h>

#include <istok/ecs.hpp>
#include <istok/logging.hpp>

#include "winapi/base/message.hpp"


namespace Istok::GUI::WinAPI {

class Dispatcher2 {
public:
    using Handler = std::move_only_function<
        LRESULT(ECS::Entity, const WindowMessage&) noexcept>;

    Dispatcher2(Handler&& defaultHandler)
    : defaultHandler_(std::move(defaultHandler)) {}

    ~Dispatcher2() = default;

    Dispatcher2(const Dispatcher2&) = delete;
    Dispatcher2& operator=(const Dispatcher2&) = delete;
    Dispatcher2(Dispatcher2&&) = default;
    Dispatcher2& operator=(Dispatcher2&&) = default;

    LRESULT handleMessage(
        ECS::Entity entity, const WindowMessage& message) noexcept;
    void setHandler(UINT msg, Handler&& handler) noexcept;

private:
    CLASS_WITH_LOGGER_PREFIX("Istok.GUI.WinAPI.Dispatcher2", "WinAPI: ");
    std::unordered_map<UINT, Handler> handlers_;
    Handler defaultHandler_;
};

}  // namespace Istok::WinAPI

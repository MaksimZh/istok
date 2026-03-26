// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "dispatcher2.hpp"

#include <windows.h>

#include <istok/ecs.hpp>
#include "winapi/base/message.hpp"

namespace Istok::GUI::WinAPI {

LRESULT Dispatcher2::handleMessage(
    ECS::Entity entity, const WindowMessage& message
) noexcept {
    auto it = handlers_.find(message.msg);
    if (it == handlers_.end()) {
        return defaultHandler_(entity, message);
    }
    LOG_TRACE("{}:{}", entity, message);
    return it->second(entity, message);
}

void Dispatcher2::setHandler(UINT msg, Handler&& func) noexcept {
    handlers_[msg] = std::move(func);
}

}  // namespace Istok::GUI::WinAPI

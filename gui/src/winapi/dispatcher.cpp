// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "dispatcher.hpp"

#include <optional>

#include <istok/ecs.hpp>
#include "message.hpp"

namespace Istok::GUI::WinAPI {

LRESULT Dispatcher::handleMessage(
    ECS::Entity entity, const WindowMessage& message
) noexcept {
    auto it = handlers_.find(message.msg);
    if (it == handlers_.end()) {
        return winapi_.defWindowProc(message);
    }
    LOG_TRACE("{}:{}", entity, message);
    auto optResult = it->second(entity, message);
    if (!optResult.has_value()) {
        return winapi_.defWindowProc(message);
    }
    return optResult.value();
}

void Dispatcher::setHandler(UINT msg, Handler&& func) noexcept {
    handlers_[msg] = std::move(func);
}

}  // namespace Istok::GUI::WinAPI

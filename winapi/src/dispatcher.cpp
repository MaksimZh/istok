// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "istok/winapi/dispatcher.hpp"

namespace Istok::WinAPI {

LRESULT WindowMessageDispatcher::handleMessage(
    ECS::Entity entity, WinAPI::WindowMessage message
) noexcept {
    auto it = handlers_.find(message.msg);
    if (it == handlers_.end()) {
        return WinAPI::handleMessageByDefault(message);
    }
    LOG_TRACE("@{}:{}", entity.index(), WinAPI::formatMessage(message));
    return it->second(ecs_, entity, message);
}

void WindowMessageDispatcher::setHandler(UINT msg, Handler&& func) {
    handlers_[msg] = std::move(func);
}

}  // namespace Istok::WinAPI

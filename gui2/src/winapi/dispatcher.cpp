// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "dispatcher.hpp"

namespace Istok::GUI::WinAPI {

LRESULT Dispatcher::handleMessage(
    ECS::Entity entity, const WinAPI::WindowMessage& message
) noexcept {
    auto it = handlers_.find(message.msg);
    if (it == handlers_.end()) {
        return winapi_.defWindowProc(message);
    }
    LOG_TRACE("@{}:{}", entity, message);
    return it->second(ecs_, entity, message);
}

void Dispatcher::setHandler(UINT msg, Handler&& func) {
    handlers_[msg] = std::move(func);
}

}  // namespace Istok::GUI::WinAPI

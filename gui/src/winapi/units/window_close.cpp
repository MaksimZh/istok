// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "window_close.hpp"

#include <optional>

#include <istok/ecs.hpp>
#include <istok/logging.hpp>

#include "winapi/base/dispatcher.hpp"
#include "winapi/base/environment.hpp"
#include "winapi/base/winapi_delegate.hpp"


namespace Istok::GUI::WinAPI {

namespace {

std::optional<LRESULT> closeHandler(
    ECS::ECSManager& ecs, const WindowEntityMessage& message
) noexcept {
    if (!ecs.has<EventHandlers::Close>(message.entity)) {
        return std::nullopt;
    }
    ecs.get<EventHandlers::Close>(message.entity).func();
    return 0;
}

}  // namespace


bool setupWindowClose(ECS::ECSManager& ecs) {
   return runInEnvironment(
        ecs,
        [](
            ECS::ECSManager& ecs,
            WinAPIDelegate& winapi,
            Dispatcher& dispatcher
        ) {
            dispatcher.setHandler(
                WM_CLOSE,
                [&ecs](const WindowEntityMessage& message) noexcept {
                    return closeHandler(ecs, message); });
            return true;
        });
}

}  // namespace Istok::GUI::WinAPI

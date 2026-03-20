// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "window_size.hpp"

#include <optional>

#include <istok/ecs.hpp>
#include <istok/logging.hpp>

#include "winapi/base/dispatcher.hpp"
#include "winapi/base/environment.hpp"
#include "winapi/base/winapi_delegate.hpp"


namespace Istok::GUI::WinAPI {

namespace {

std::optional<LRESULT> sizeHandler(
    ECS::ECSManager& ecs, const WindowEntityMessage& message
) noexcept {
    if (!ecs.has<NewWindowMarker>(message.entity)) {
        ecs.iterate();
    }
    return std::nullopt;
}

}  // namespace


bool setupWindowSize(ECS::ECSManager& ecs) {
    return runInEnvironment(
        ecs,
        [](
            ECS::ECSManager& ecs,
            WinAPIDelegate& winapi,
            Dispatcher& dispatcher
        ) {
            dispatcher.setHandler(
                WM_SIZE,
                [&ecs](const WindowEntityMessage& message) noexcept {
                    return sizeHandler(ecs, message); });
            return true;
        });
}

}  // namespace Istok::GUI::WinAPI

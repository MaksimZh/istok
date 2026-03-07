// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "events.hpp"

#include <memory>

#include <istok/ecs.hpp>

#include "istok/gui/base.hpp"
#include "delegate.hpp"
#include "dispatcher.hpp"
#include "message.hpp"

namespace Istok::GUI::WinAPI {

namespace {

LRESULT closeHandler(
    WinAPIDelegate& winapi, ECS::ECSManager& ecs,
    ECS::Entity entity, WindowMessage message
) noexcept {
    assert(message.msg == WM_CLOSE);
    if (!ecs.has<EventHandlers::Close>(entity)) {
        return winapi.defWindowProc(message);
    }
    ecs.get<EventHandlers::Close>(entity).func();
    return 0;
}

LRESULT sizeHandler(
    WinAPIDelegate& winapi, ECS::ECSManager& ecs,
    ECS::Entity entity, WindowMessage message
) noexcept {
    assert(message.msg == WM_SIZE);
    if (!ecs.has<NewWindowMarker>(entity)) {
        ecs.iterate();
    }
    return winapi.defWindowProc(message);
}

}  // namespace


WindowMessageHandlerGenerator setupDispatcher(
    WinAPIDelegate& winapi, ECS::ECSManager& ecs, ECS::Entity master
) {
    auto dispatcher = std::make_unique<Dispatcher>(winapi, ecs);
    dispatcher->setHandler(WM_CLOSE, closeHandler);
    dispatcher->setHandler(WM_SIZE, sizeHandler);
    auto* dispatcherPtr = dispatcher.get();
    ecs.insert(master, std::move(dispatcher));
    return WindowMessageHandlerGenerator{
        [dispatcherPtr](ECS::Entity entity) noexcept {
            return WindowMessageHandler{
                [entity, dispatcherPtr](
                    const WindowMessage& message
                ) noexcept {
                    return dispatcherPtr->handleMessage(entity, message);
                }};
        }};
}

}  // namespace Istok::GUI::WinAPI

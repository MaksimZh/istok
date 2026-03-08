// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "dispatcher_setup.hpp"

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


void setupDispatcher(
    WinAPIDelegate& winapi, ECS::ECSManager& ecs, ECS::Entity master
) {
    auto dispatcher = std::make_unique<Dispatcher>(winapi, ecs);
    dispatcher->setHandler(WM_CLOSE, closeHandler);
    dispatcher->setHandler(WM_SIZE, sizeHandler);
    auto* dispatcherPtr = dispatcher.get();
    ecs.insert(master, std::move(dispatcher));
}

}  // namespace Istok::GUI::WinAPI

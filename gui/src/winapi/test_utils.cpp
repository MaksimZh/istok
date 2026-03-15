// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "winapi/test_utils.hpp"

#include <istok/ecs.hpp>

namespace Istok::GUI::WinAPI {

MockWinAPI& setupMockWinAPI(ECS::ECSManager& ecs, ECS::Entity master) {
    auto winapiContainer = std::make_unique<MockWinAPI>();
    MockWinAPI& winapi = *winapiContainer;
    ecs.insert(
        master, std::unique_ptr<WinAPIDelegate>{std::move(winapiContainer)});
    return winapi;
}

MockWinAPI& setupMockWinAPI(ECS::ECSManager& ecs) {
    return setupMockWinAPI(ecs, ecs.createEntity());
}

}  // namespace Istok::GUI::WinAPI

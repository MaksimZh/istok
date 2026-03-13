// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "winapi/test_utils.hpp"

#include <istok/ecs.hpp>

namespace Istok::GUI::WinAPI {

MockWinAPI& setupMockWinAPI(ECS::ECSManager& ecs) {
    const ECS::Entity master = ecs.createEntity();
    auto winapiContainer = std::make_unique<MockWinAPI>();
    MockWinAPI& winapi = *winapiContainer;
    ecs.insert(
        master, std::unique_ptr<WinAPIDelegate>{std::move(winapiContainer)});
    return winapi;
}

}  // namespace Istok::GUI::WinAPI

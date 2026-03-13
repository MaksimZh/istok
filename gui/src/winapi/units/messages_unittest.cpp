// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "winapi/units/messages.hpp"

#include <windows.h>

#include <istok/ecs.hpp>

#include "winapi/base/delegate.hpp"
#include "winapi/base/dispatcher.hpp"
#include "winapi/test_utils.hpp"

using namespace Istok;
using namespace Istok::GUI;
using namespace Istok::GUI::WinAPI;
using trompeloeil::_;


TEST_CASE("Messages", "[unit][winapi]") {
    ECS::ECSManager ecs;
    const ECS::Entity master = ecs.createEntity();
    REQUIRE_FALSE(setupMessages(ecs));

    ecs.insert(master, std::unique_ptr<WinAPIDelegate>());
    REQUIRE_FALSE(setupMessages(ecs));

    auto winapiContainer = std::make_unique<MockWinAPI>();
    MockWinAPI& winapi = *winapiContainer;
    ecs.insert(
        master, std::unique_ptr<WinAPIDelegate>{std::move(winapiContainer)});
    ecs.insert(master, std::make_unique<Dispatcher>(winapi));
    REQUIRE(setupMessages(ecs));

    REQUIRE(ecs.has<std::unique_ptr<Dispatcher>>(master));
    REQUIRE(ecs.get<std::unique_ptr<Dispatcher>>(master));
    {
        const MSG msg{reinterpret_cast<HWND>(1), WM_SIZE, 1, 2, 0, 0};
        REQUIRE_CALL(winapi, getMessage()).RETURN(msg)
            .LR_SIDE_EFFECT([&winapi, &ecs]{
                // Nested runs must skip all actions
                FORBID_CALL(winapi, getMessage());
                FORBID_CALL(winapi, dispatchMessage(_));
                ecs.iterate();
            }());
        REQUIRE_CALL(winapi, dispatchMessage(msg));
        ecs.iterate();
    }
}

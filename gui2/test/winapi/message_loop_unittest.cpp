// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "winapi/message_loop.hpp"

#include <windows.h>

#include <istok/ecs.hpp>

#include "istok/gui/callback_types.hpp"
#include "utils.hpp"

using namespace Istok;
using namespace Istok::GUI;
using namespace Istok::GUI::WinAPI;
using trompeloeil::_;


TEST_CASE("Message loop - setup", "[unit][winapi]") {
    MockWinAPI winapi;
    ECS::ECSManager ecs;
    ECS::Entity master = ecs.createEntity();
    QuitCallback quit([]() noexcept {});
    ECS::System sys =
        createMessageLoopSystem(&winapi, std::move(quit), master);

    {
        REQUIRE_CALL(winapi, getMessage(_))
            .LR_SIDE_EFFECT([&winapi, &ecs, &sys]{
                // Nested runs must skip all actions
                FORBID_CALL(winapi, getMessage(_));
                FORBID_CALL(winapi, dispatchMessage(_));
                sys(ecs);
            }())
            .SIDE_EFFECT(_1.message = WM_SIZE);
        REQUIRE_CALL(winapi, dispatchMessage(_))
            .WITH(_1.message == WM_SIZE);
        sys(ecs);
    }
}

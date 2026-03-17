// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "winapi/core/message_loop.hpp"

#include <windows.h>

#include <istok/ecs.hpp>

#include "winapi/test_utils.hpp"

using namespace Istok;
using namespace Istok::GUI;
using namespace Istok::GUI::WinAPI;
using trompeloeil::_;


TEST_CASE("Messages - run", "[unit][winapi]") {
    ECS::ECSManager ecs;
    MockWinAPI& winapi = setupMockWinAPI(ecs);
    REQUIRE(setupMessages(ecs));

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

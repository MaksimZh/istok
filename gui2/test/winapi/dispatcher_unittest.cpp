// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "winapi/dispatcher.hpp"

#include <istok/ecs.hpp>

#include "utils.hpp"

using namespace Istok;
using namespace Istok::GUI::WinAPI;

TEST_CASE("WindowMessageDispatcher - handlers", "[unit][winapi]") {
    ECS::ECSManager ecs;
    MockWinAPI winapi;
    WindowMessageDispatcher dispatcher(winapi, ecs);
    WindowMessage message{
        reinterpret_cast<HWND>(1), WM_SIZE, SIZE_MAXIMIZED, MAKELPARAM(5, 7)};
    auto entity = ecs.createEntity();
    {
        REQUIRE_CALL(winapi, defWindowProc(message)).RETURN(42);
        REQUIRE(dispatcher.handleMessage(entity, message) == 42);
    }
}

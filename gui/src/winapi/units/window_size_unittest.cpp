// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "winapi/units/window_size.hpp"

#include <windows.h>

#include <istok/ecs.hpp>

#include "istok/gui/base.hpp"
#include "winapi/base/test_utils.hpp"
#include "winapi/base/window.hpp"
#include "winapi/core/window_life.hpp"

using namespace Istok;
using namespace Istok::GUI;
using namespace Istok::GUI::WinAPI;

namespace {

struct MockCall {
    MAKE_MOCK0(call, void(), noexcept);
};

}  // namespace

TEST_CASE("Window - size", "[unit][winapi]") {
    FakeWindowsMockWinAPI winapi;
    ECS::ECSManager ecs;
    setupWinAPIProxy(ecs, ecs.createEntity(), winapi);
    REQUIRE(setupWindowLife(ecs));
    REQUIRE(setupWindowSize(ecs));

    const ECS::Entity a = ecs.createEntity();
    ecs.insert(a, CreateWindowMarker{});
    ecs.insert(a, WindowLocation{});
    ecs.iterate();
    const WindowMessage sizeMessage{
        ecs.get<Window>(a).getHWnd(), WM_SIZE,
        SIZE_MAXIMIZED, MAKELPARAM(5, 7)};

    MockCall iteration;
    ecs.addLoopSystem([&iteration](ECS::ECSManager& ecs) noexcept {
        iteration.call(); });

    REQUIRE(ecs.has<NewWindowMarker>(a));
    {
        REQUIRE_CALL(winapi, defWindowProc(sizeMessage)).RETURN(43);
        FORBID_CALL(iteration, call());
        REQUIRE(winapi.handleMessage(sizeMessage) == 43);
    }

    ecs.remove<NewWindowMarker>(a);
    {
        REQUIRE_CALL(winapi, defWindowProc(sizeMessage)).RETURN(42);
        REQUIRE_CALL(iteration, call());
        REQUIRE(winapi.handleMessage(sizeMessage) == 42);
    }
}

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

    MockCall loop1;
    MockCall loop2;
    ecs.addLoopSystem([&loop1](ECS::ECSManager& ecs) noexcept {
        loop1.call(); });
    ecs.addLoopSystem([&loop2](ECS::ECSManager& ecs) noexcept {
        loop2.call(); });

    REQUIRE(ecs.has<NewWindowMarker>(a));
    {
        REQUIRE_CALL(loop1, call());
        REQUIRE_CALL(loop2, call())
            .LR_SIDE_EFFECT([&]() {
                REQUIRE_CALL(winapi, defWindowProc(sizeMessage)).RETURN(43);
                FORBID_CALL(loop1, call());
                REQUIRE(winapi.handleMessage(sizeMessage) == 43);
            });
        ecs.iterate();
    }

    REQUIRE(!ecs.has<NewWindowMarker>(a));
    {
        REQUIRE_CALL(loop1, call());
        REQUIRE_CALL(loop2, call())
            .LR_SIDE_EFFECT([&]() {
                REQUIRE_CALL(winapi, defWindowProc(sizeMessage)).RETURN(42);
                REQUIRE_CALL(loop1, call());
                REQUIRE(winapi.handleMessage(sizeMessage) == 42);
            });
        ecs.iterate();
    }
}

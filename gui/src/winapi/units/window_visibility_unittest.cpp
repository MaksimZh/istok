// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "winapi/units/window_visibility.hpp"

#include <windows.h>

#include <istok/ecs.hpp>

#include "istok/gui/base.hpp"
#include "winapi/base/test_utils.hpp"
#include "winapi/base/window.hpp"
#include "winapi/core/window_life.hpp"

using namespace Istok;
using namespace Istok::GUI;
using namespace Istok::GUI::WinAPI;
using trompeloeil::_;


namespace {

struct MockClose {
    MAKE_MOCK1(call,
        std::optional<LRESULT>(const WindowEntityMessage&), noexcept);
};

}  // namespace


TEST_CASE("Window - visibility", "[unit][winapi]") {
    FakeWindowsMockWinAPI winapi;
    ECS::ECSManager ecs;
    setupWinAPIProxy(ecs, ecs.createEntity(), winapi);
    REQUIRE(setupWindowLife(ecs));
    REQUIRE(setupWindowVisibility(ecs));

    const ECS::Entity a = ecs.createEntity();
    ecs.insert(a, CreateWindowMarker{});
    ecs.insert(a, WindowLocation{});
    const ECS::Entity b = ecs.createEntity();
    ecs.insert(b, CreateWindowMarker{});
    ecs.insert(b, WindowLocation{});
    ecs.insert(b, ShowWindowMarker{});
    const ECS::Entity c = ecs.createEntity();
    ecs.insert(c, ShowWindowMarker{});
    {
        REQUIRE_CALL(winapi, showWindow(_))
            .LR_WITH(_1 == ecs.get<Window>(b).getHWnd());
        ecs.iterate();
    }
}

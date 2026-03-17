// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "winapi/units/window_visibility.hpp"

#include <windows.h>

#include <istok/ecs.hpp>

#include "istok/gui/base.hpp"
#include "winapi/base/window_test_utils.hpp"
#include "winapi/test_utils.hpp"
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
    ECS::ECSManager ecs;
    MockWinAPI& winapi = setupMockWinAPI(ecs);
    REQUIRE(setupWindowLife(ecs));
    REQUIRE(setupWindowVisibility(ecs));

    const ECS::Entity a = ecs.createEntity();
    const ECS::Entity b = ecs.createEntity();
    const ECS::Entity c = ecs.createEntity();

    const HWND hWndA = reinterpret_cast<HWND>(1);
    const HWND hWndB = reinterpret_cast<HWND>(2);
    const Rect<int> rectA{1, 1, 1, 1};
    const Rect<int> rectB{2, 2, 2, 2};
    ecs.insert(a, CreateWindowMarker{});
    ecs.insert(a, WindowLocation{rectA});
    ecs.insert(b, CreateWindowMarker{});
    ecs.insert(b, WindowLocation{rectB});
    ecs.insert(b, ShowWindowMarker{});
    ecs.insert(c, ShowWindowMarker{});
    {
        REQUIRE_CREATE_WINDOW(winapi, rectA, hWndA);
        REQUIRE_CREATE_WINDOW(winapi, rectB, hWndB);
        REQUIRE_CALL(winapi, showWindow(hWndB));
        ecs.iterate();
    }

    {
        ALLOW_DESTROY_WINDOWS(winapi);
        ecs.clear();
    }
}

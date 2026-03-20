// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "winapi/units/window_visibility.hpp"

#include <windows.h>

#include <istok/ecs.hpp>

#include "istok/gui/base.hpp"
#include "winapi/base/dispatcher.hpp"
#include "winapi/base/winapi_delegate.hpp"
#include "winapi/base/fake_windows.hpp"
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
    ECS::ECSManager ecs;
    ECS::Entity master = ecs.createEntity();
    auto winAPIContainer = std::make_unique<FakeWindowsMockWinAPI>();
    FakeWindowsMockWinAPI& winapi = *winAPIContainer;
    ecs.insert(
        master, std::unique_ptr<WinAPIDelegate>(std::move(winAPIContainer)));
    REQUIRE(setupWindowLife(ecs));
    REQUIRE(setupWindowVisibility(ecs));

    const ECS::Entity a = ecs.createEntity();
    const ECS::Entity b = ecs.createEntity();
    const ECS::Entity c = ecs.createEntity();

    const Rect<int> rectA{1, 1, 1, 1};
    const Rect<int> rectB{2, 2, 2, 2};
    ecs.insert(a, CreateWindowMarker{});
    ecs.insert(a, WindowLocation{rectA});
    ecs.insert(b, CreateWindowMarker{});
    ecs.insert(b, WindowLocation{rectB});
    ecs.insert(b, ShowWindowMarker{});
    ecs.insert(c, ShowWindowMarker{});
    HWND hWndB = nullptr;
    {
        REQUIRE_CALL(winapi, showWindow(_)).LR_WITH(hWndB = _1);
        ecs.iterate();
    }
    REQUIRE(winapi.windowsCount() == 2);
    REQUIRE(ecs.get<Window>(b).getHWnd() == hWndB);
}

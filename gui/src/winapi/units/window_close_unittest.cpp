// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#define NOMINMAX
#include "winapi/units/window_close.hpp"

#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
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

struct MockCall {
    MAKE_MOCK0(call, void(), noexcept);
};

}  // namespace

TEST_CASE("Window - close", "[unit][winapi]") {
    FakeWindowsMockWinAPI winapi;
    ECS::ECSManager ecs;
    setupWinAPIProxy(ecs, ecs.createEntity(), winapi);
    REQUIRE(setupWindowLife(ecs));
    REQUIRE(setupWindowClose(ecs));

    const ECS::Entity a = ecs.createEntity();
    ecs.insert(a, CreateWindowMarker{});
    ecs.insert(a, WindowLocation{});
    ecs.iterate();
    const WindowMessage closeMessage{
        ecs.get<Window>(a).getHWnd(), WM_CLOSE, NULL, NULL};
    {
        REQUIRE_CALL(winapi, defWindowProc(closeMessage)).RETURN(42);
        REQUIRE(winapi.handleMessage(closeMessage) == 42);
    }

    MockCall closeHandler;
    ecs.insert(a, EventHandlers::Close{
        [&closeHandler]() noexcept { closeHandler.call(); }});
    {
        REQUIRE_CALL(closeHandler, call());
        FORBID_CALL(winapi, defWindowProc(_));
        REQUIRE(winapi.handleMessage(closeMessage) == 0);
    }
}

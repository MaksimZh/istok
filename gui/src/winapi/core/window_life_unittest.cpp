// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "winapi/core/window_life.hpp"

#include <windows.h>

#include <istok/ecs.hpp>

#include "istok/gui/base.hpp"
#include "winapi/base/dispatcher.hpp"
#include "winapi/base/window_test_utils.hpp"
#include "winapi/base/window.hpp"
#include "winapi/test_utils.hpp"

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

TEST_CASE("Window - life", "[unit][winapi]") {
    ECS::ECSManager ecs;
    ECS::Entity master = ecs.createEntity();
    MockWinAPI& winapi = setupMockWinAPI(ecs, master);
    REQUIRE(setupWindowLife(ecs));

    MockClose close;
    ecs.get<std::unique_ptr<Dispatcher>>(master)
        ->setHandler(
            WM_CLOSE,
            [&close](const WindowEntityMessage& message) noexcept {
                return close.call(message);
            });

    const ECS::Entity a = ecs.createEntity();
    const ECS::Entity b = ecs.createEntity();
    {
        FORBID_CALL(winapi, createWindow(_));
        ecs.iterate();
    }

    const HWND hWndA = reinterpret_cast<HWND>(1);
    const Rect<int> rectA{1, 2, 3, 4};
    ecs.insert(a, CreateWindowMarker{});
    ecs.insert(a, WindowLocation{rectA});
    WindowMessageHandler* handlerA = nullptr;
    {
        REQUIRE_CREATE_WINDOW_HANDLER(winapi, rectA, hWndA, handlerA);
        ecs.iterate();
    }
    REQUIRE(handlerA);
    REQUIRE(ecs.has<NewWindowMarker>(a));
    REQUIRE(ecs.has<Window>(a));
    REQUIRE(ecs.get<Window>(a).getHWnd() == hWndA);
    {
        const WindowMessage message{hWndA, WM_CLOSE, 11, 12};
        const LRESULT result = 42;
        REQUIRE_CALL(close, call(
            WindowEntityMessage{a, message.wParam, message.lParam}))
            .RETURN(result);
        REQUIRE((*handlerA)(message) == result);
    }

    const HWND hWndB = reinterpret_cast<HWND>(2);
    const Rect<int> rectB{2, 3, 4, 5};
    ecs.insert(b, CreateWindowMarker{});
    ecs.insert(b, WindowLocation{rectB});
    WindowMessageHandler* handlerB = nullptr;
    {
        REQUIRE_CREATE_WINDOW_HANDLER(winapi, rectB, hWndB, handlerB);
        ecs.iterate();
    }
    REQUIRE(handlerB);
    REQUIRE(!ecs.has<NewWindowMarker>(a));
    REQUIRE(ecs.has<NewWindowMarker>(b));
    REQUIRE(ecs.has<Window>(b));
    REQUIRE(ecs.get<Window>(b).getHWnd() == hWndB);
    {
        const WindowMessage message{hWndB, WM_CLOSE, 13, 14};
        const LRESULT result = 43;
        REQUIRE_CALL(close, call(
            WindowEntityMessage{b, message.wParam, message.lParam}))
            .RETURN(result);
        REQUIRE((*handlerB)(message) == result);
    }

    {
        REQUIRE_DESTROY_WINDOW(winapi, hWndB);
        REQUIRE_DESTROY_WINDOW(winapi, hWndA);
        ecs.clear();
    }
}

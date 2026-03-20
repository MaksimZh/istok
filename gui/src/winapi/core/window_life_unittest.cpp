// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "winapi/core/window_life.hpp"

#include <windows.h>

#include <istok/ecs.hpp>

#include "istok/gui/base.hpp"
#include "winapi/base/dispatcher.hpp"
#include "winapi/base/fake_windows.hpp"
#include "winapi/base/window.hpp"
#include "winapi/test_utils.hpp"
#include "winapi/base/winapi_proxy.hpp"

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
    auto winAPIContainer = std::make_unique<FakeWindowsMockWinAPI>();
    FakeWindowsMockWinAPI& winapi = *winAPIContainer;
    ecs.insert(
        master,
        std::unique_ptr<WinAPIDelegate>(
            std::make_unique<WinAPIProxy>(winAPIContainer.get())));
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
    ecs.iterate();
    REQUIRE(winapi.windowsCount() == 0);

    const Rect<int> rectA{1, 2, 3, 4};
    ecs.insert(a, CreateWindowMarker{});
    ecs.insert(a, WindowLocation{rectA});
    ecs.iterate();
    REQUIRE(winapi.windowsCount() == 1);
    REQUIRE(ecs.has<NewWindowMarker>(a));
    REQUIRE(ecs.has<Window>(a));
    HWND hWndA = ecs.get<Window>(a).getHWnd();
    auto handlerA = winapi.getWindowMessageHandler(hWndA);
    REQUIRE(handlerA);
    {
        const WindowMessage message{hWndA, WM_CLOSE, 11, 12};
        const LRESULT result = 42;
        REQUIRE_CALL(close, call(
            WindowEntityMessage{a, message.wParam, message.lParam}))
            .RETURN(result);
        REQUIRE((*handlerA)(message) == result);
    }

    const Rect<int> rectB{2, 3, 4, 5};
    ecs.insert(b, CreateWindowMarker{});
    ecs.insert(b, WindowLocation{rectB});
    ecs.iterate();
    REQUIRE(winapi.windowsCount() == 2);
    REQUIRE(!ecs.has<NewWindowMarker>(a));
    REQUIRE(ecs.has<NewWindowMarker>(b));
    REQUIRE(ecs.has<Window>(b));
    HWND hWndB = ecs.get<Window>(b).getHWnd();
    auto handlerB = winapi.getWindowMessageHandler(hWndB);
    {
        const WindowMessage message{hWndB, WM_CLOSE, 13, 14};
        const LRESULT result = 43;
        REQUIRE_CALL(close, call(
            WindowEntityMessage{b, message.wParam, message.lParam}))
            .RETURN(result);
        REQUIRE((*handlerB)(message) == result);
    }

    ecs.clear();
    REQUIRE(winapi.windowsCount() == 0);
}

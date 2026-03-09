// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "winapi/window_life.hpp"

#include <windows.h>

#include <istok/ecs.hpp>

#include "istok/gui/base.hpp"
#include "winapi/dispatcher.hpp"
#include "winapi/window.hpp"
#include "utils.hpp"

using namespace Istok;
using namespace Istok::GUI;
using namespace Istok::GUI::WinAPI;
using trompeloeil::_;

namespace {

struct MockClose {
    MAKE_MOCK4(
        call,
        LRESULT(WinAPIDelegate&, ECS::ECSManager&, ECS::Entity, WindowMessage),
        noexcept);
};

}  // namespace

TEST_CASE("Window - life", "[unit][winapi]") {
    auto* pECS = new ECS::ECSManager;
    ECS::ECSManager& ecs = *pECS;
    const ECS::Entity master = ecs.createEntity();
    REQUIRE_FALSE(setupWindowLife(ecs));

    ecs.insert(master, std::unique_ptr<WinAPIDelegate>());
    REQUIRE_FALSE(setupWindowLife(ecs));

    auto winapiContainer = std::make_unique<MockWinAPI>();
    MockWinAPI& winapi = *winapiContainer;
    ecs.insert(
        master, std::unique_ptr<WinAPIDelegate>{std::move(winapiContainer)});
    REQUIRE_FALSE(setupWindowLife(ecs));

    ecs.insert(master, std::unique_ptr<Dispatcher>());
    REQUIRE_FALSE(setupWindowLife(ecs));

    ecs.insert(master, std::make_unique<Dispatcher>(winapi, ecs));
    REQUIRE(setupWindowLife(ecs));

    MockClose close;
    ecs.get<std::unique_ptr<Dispatcher>>(master)
        ->setHandler(
            WM_CLOSE,
            [&close](
                WinAPIDelegate& winapi, ECS::ECSManager& ecs,
                ECS::Entity entity, const WindowMessage& message
            ) noexcept {
                return close.call(winapi, ecs, entity, message);
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
        REQUIRE_CALL(winapi, createWindow(rectA)).RETURN(hWndA);
        REQUIRE_CALL(winapi, setRawUserPointer(hWndA, _))
            .LR_SIDE_EFFECT(
                handlerA = reinterpret_cast<WindowMessageHandler*>(_2));
        ecs.iterate();
    }
    REQUIRE(handlerA);
    REQUIRE(ecs.has<NewWindowMarker>(a));
    REQUIRE(ecs.has<Window>(a));
    REQUIRE(ecs.get<Window>(a).getHWnd() == hWndA);
    {
        const WindowMessage message{hWndA, WM_CLOSE, 0, 0};
        const LRESULT result = 42;
        REQUIRE_CALL(close, call(_, _, a, message)).RETURN(result);
        REQUIRE((*handlerA)(message) == result);
    }

    const HWND hWndB = reinterpret_cast<HWND>(2);
    const Rect<int> rectB{2, 3, 4, 5};
    ecs.insert(b, CreateWindowMarker{});
    ecs.insert(b, WindowLocation{rectB});
    WindowMessageHandler* handlerB = nullptr;
    {
        REQUIRE_CALL(winapi, createWindow(rectB)).RETURN(hWndB);
        REQUIRE_CALL(winapi, setRawUserPointer(hWndB, _))
            .LR_SIDE_EFFECT(
                handlerB = reinterpret_cast<WindowMessageHandler*>(_2));
        ecs.iterate();
    }
    REQUIRE(handlerB);
    REQUIRE(!ecs.has<NewWindowMarker>(a));
    REQUIRE(ecs.has<NewWindowMarker>(b));
    REQUIRE(ecs.has<Window>(b));
    REQUIRE(ecs.get<Window>(b).getHWnd() == hWndB);
    {
        const WindowMessage message{hWndB, WM_CLOSE, 0, 0};
        const LRESULT result = 43;
        REQUIRE_CALL(close, call(_, _, b, message)).RETURN(result);
        REQUIRE((*handlerB)(message) == result);
    }

    {
        REQUIRE_CALL(winapi, setRawUserPointer(hWndB, NULL));
        REQUIRE_CALL(winapi, destroyWindow(hWndB));
        REQUIRE_CALL(winapi, setRawUserPointer(hWndA, NULL));
        REQUIRE_CALL(winapi, destroyWindow(hWndA));
        delete pECS;
    }
}

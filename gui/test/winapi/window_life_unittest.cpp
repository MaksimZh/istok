// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "winapi/window_life.hpp"

#include <windows.h>

#include <istok/ecs.hpp>

#include "istok/gui/base.hpp"
#include "winapi/dispatcher.hpp"
#include "utils.hpp"

using namespace Istok;
using namespace Istok::GUI;
using namespace Istok::GUI::WinAPI;
using trompeloeil::_;

namespace {

struct MockCall {
    MAKE_MOCK0(call, void(), noexcept);
};

}  // namespace

TEST_CASE("Window - life", "[unit][winapi]") {
    MockWinAPI winapi;
    auto* pECS = new ECS::ECSManager;
    ECS::ECSManager& ecs = *pECS;

    const ECS::Entity deleted = ecs.createEntity();
    ecs.deleteEntity(deleted);
    REQUIRE(setupWindowLife(winapi, ecs, deleted).error_or("") ==
        "Invalid master entity.");

    const ECS::Entity master = ecs.createEntity();
    REQUIRE(setupWindowLife(winapi, ecs, master).error_or("") ==
        "No Dispatcher found on master entity.");

    ecs.insert(master, std::unique_ptr<Dispatcher>());
    REQUIRE(setupWindowLife(winapi, ecs, master).error_or("") ==
        "Empty Dispatcher found on master entity.");

    ecs.insert(master, std::make_unique<Dispatcher>(winapi, ecs));
    REQUIRE(setupWindowLife(winapi, ecs, master));

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
    WindowMessageHandler* handlerA;
    {
        REQUIRE_CALL(winapi, createWindow(rectA)).RETURN(hWndA);
        REQUIRE_CALL(winapi, setRawUserPointer(hWndA, _))
            .LR_SIDE_EFFECT(
                handlerA = reinterpret_cast<WindowMessageHandler*>(_1));
        ecs.iterate();
    }
    REQUIRE(ecs.has<NewWindowMarker>(a));
    // TODO: make Dispatcher an interface, make mock for it and test handler

    const HWND hWndB = reinterpret_cast<HWND>(2);
    const Rect<int> rectB{2, 3, 4, 5};
    ecs.insert(b, CreateWindowMarker{});
    ecs.insert(b, WindowLocation{rectB});
    WindowMessageHandler* handlerB;
    {
        REQUIRE_CALL(winapi, createWindow(rectB)).RETURN(hWndB);
        REQUIRE_CALL(winapi, setRawUserPointer(hWndB, _))
            .LR_SIDE_EFFECT(
                handlerB = reinterpret_cast<WindowMessageHandler*>(_1));
        ecs.iterate();
    }
    REQUIRE(!ecs.has<NewWindowMarker>(a));
    REQUIRE(ecs.has<NewWindowMarker>(b));
    // TODO: make Dispatcher an interface, make mock for it and test handler

    {
        REQUIRE_CALL(winapi, setRawUserPointer(hWndB, NULL));
        REQUIRE_CALL(winapi, destroyWindow(hWndB));
        REQUIRE_CALL(winapi, setRawUserPointer(hWndA, NULL));
        REQUIRE_CALL(winapi, destroyWindow(hWndA));
        delete pECS;
    }
}

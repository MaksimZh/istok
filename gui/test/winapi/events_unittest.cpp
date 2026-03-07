// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "winapi/dispatcher_setup.hpp"

#include <windows.h>

#include <istok/ecs.hpp>

#include "istok/gui/base.hpp"
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

TEST_CASE("Events - handling", "[unit][winapi]") {
    MockWinAPI winapi;
    ECS::ECSManager ecs;
    const ECS::Entity master = ecs.createEntity();
    const WindowMessage closeMessage{
        reinterpret_cast<HWND>(1), WM_CLOSE, NULL, NULL};
    const WindowMessage sizeMessage{
        reinterpret_cast<HWND>(1), WM_SIZE,
        SIZE_MAXIMIZED, MAKELPARAM(5, 7)};

    WindowMessageHandlerGenerator handlerGenerator =
        setupDispatcher(winapi, ecs, master);
    const ECS::Entity entity = ecs.createEntity();
    WindowMessageHandler handler = handlerGenerator(entity);
    {
        REQUIRE_CALL(winapi, defWindowProc(closeMessage)).RETURN(42);
        REQUIRE(handler(closeMessage) == 42);
    }

    MockCall closeHandler;
    ecs.insert(entity, EventHandlers::Close{
        [&closeHandler]() noexcept { closeHandler.call(); }});
    {
        REQUIRE_CALL(closeHandler, call());
        FORBID_CALL(winapi, defWindowProc(_));
        REQUIRE(handler(closeMessage) == 0);
    }
    {
        REQUIRE_CALL(winapi, defWindowProc(sizeMessage)).RETURN(42);
        REQUIRE(handler(sizeMessage) == 42);
    }
}


TEST_CASE("Events - size", "[unit][winapi]") {
    MockWinAPI winapi;
    ECS::ECSManager ecs;
    const ECS::Entity master = ecs.createEntity();
    const WindowMessage sizeMessage{
        reinterpret_cast<HWND>(1), WM_SIZE,
        SIZE_MAXIMIZED, MAKELPARAM(5, 7)};

    WindowMessageHandlerGenerator handlerGenerator =
        setupDispatcher(winapi, ecs, master);
    const ECS::Entity entity = ecs.createEntity();
    WindowMessageHandler handler = handlerGenerator(entity);

    MockCall iteration;
    ecs.addLoopSystem([&iteration](ECS::ECSManager& ecs) noexcept {
        iteration.call(); });
    {
        REQUIRE_CALL(iteration, call());
        REQUIRE_CALL(winapi, defWindowProc(_)).RETURN(42);
        REQUIRE(handler(sizeMessage) == 42);
    }

    ecs.insert(entity, NewWindowMarker{});
    {
        FORBID_CALL(iteration, call());
        REQUIRE_CALL(winapi, defWindowProc(_)).RETURN(42);
        REQUIRE(handler(sizeMessage) == 42);
    }
}

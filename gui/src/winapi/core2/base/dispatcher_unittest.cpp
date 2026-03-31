// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#define NOMINMAX
#include "winapi/core2/base/dispatcher.hpp"

#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include <windows.h>

#include <istok/ecs.hpp>

#include "winapi/core2/base/dispatcher.hpp"
#include "winapi/core2/base/message.hpp"

using namespace Istok;
using namespace Istok::GUI::WinAPI;
using trompeloeil::_;


namespace {

struct MockHandler {
    MAKE_MOCK2(run, LRESULT(ECS::Entity, const WindowMessage&), noexcept);

    Dispatcher::Handler get() noexcept {
        return [this](
            ECS::Entity entity, const WindowMessage& message
        ) noexcept -> LRESULT {
            return run(entity, message);
        };
    }
};

}  // namespace


TEST_CASE("Dispatcher - handlers", "[unit][winapi]") {
    const WindowMessage sizeMessage{
        reinterpret_cast<HWND>(1), WM_SIZE,
        SIZE_MAXIMIZED, MAKELPARAM(5, 7)};
    const WindowMessage cursorMessage{
        reinterpret_cast<HWND>(2), WM_SETCURSOR,
        2, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE)};

    MockHandler defaultHandler;
    Dispatcher dispatcher(defaultHandler.get());

    ECS::ECSManager ecs;
    const ECS::Entity a = ecs.createEntity();
    const ECS::Entity b = ecs.createEntity();

    {
        REQUIRE_CALL(defaultHandler, run(a, sizeMessage)).RETURN(1);
        REQUIRE(dispatcher.handleMessage(a, sizeMessage) == 1);
    }
    {
        REQUIRE_CALL(defaultHandler, run(b, cursorMessage)).RETURN(2);
        REQUIRE(dispatcher.handleMessage(b, cursorMessage) == 2);
    }

    MockHandler sizeHandler;
    dispatcher.setHandler(WM_SIZE, sizeHandler.get());

    {
        REQUIRE_CALL(sizeHandler, run(a, sizeMessage)).RETURN(1);
        FORBID_CALL(defaultHandler, run(_, _));
        REQUIRE(dispatcher.handleMessage(a, sizeMessage) == 1);
    }
    {
        REQUIRE_CALL(defaultHandler, run(b, cursorMessage)).RETURN(2);
        REQUIRE(dispatcher.handleMessage(b, cursorMessage) == 2);
    }
}
// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "winapi/base/dispatcher.hpp"

#include <optional>

#include <istok/ecs.hpp>

#include "winapi/test_utils.hpp"

using namespace Istok;
using namespace Istok::GUI::WinAPI;
using trompeloeil::_;


namespace {

struct MockHandler {
    MAKE_MOCK2(call,
        std::optional<LRESULT>(ECS::Entity, const WindowMessage&), noexcept);
};

}  // namespace


TEST_CASE("Dispatcher - handlers", "[unit][winapi]") {
    MockWinAPI winapi;
    ECS::ECSManager ecs;
    Dispatcher dispatcher(winapi);
    const WindowMessage sizeMessage{
        reinterpret_cast<HWND>(1), WM_SIZE,
        SIZE_MAXIMIZED, MAKELPARAM(5, 7)};
    const WindowMessage cursorMessage{
        reinterpret_cast<HWND>(1), WM_SETCURSOR,
        2, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE)};
    const ECS::Entity entity = ecs.createEntity();

    {
        REQUIRE_CALL(winapi, defWindowProc(sizeMessage)).RETURN(42);
        REQUIRE(dispatcher.handleMessage(entity, sizeMessage) == 42);
    }

    MockHandler handler;
    dispatcher.setHandler(
        WM_SIZE,
        [&handler](ECS::Entity entity, const WindowMessage& message) noexcept {
            return handler.call(entity, message);
        });
    {
        REQUIRE_CALL(handler, call(entity, sizeMessage)).RETURN(43);
        FORBID_CALL(winapi, defWindowProc(_));
        REQUIRE(dispatcher.handleMessage(entity, sizeMessage) == 43);
    }
    {
        REQUIRE_CALL(handler, call(entity, sizeMessage)).RETURN(std::nullopt);
        REQUIRE_CALL(winapi, defWindowProc(sizeMessage)).RETURN(44);
        REQUIRE(dispatcher.handleMessage(entity, sizeMessage) == 44);
    }
    {
        REQUIRE_CALL(winapi, defWindowProc(cursorMessage)).RETURN(45);
        REQUIRE(dispatcher.handleMessage(entity, cursorMessage) == 45);
    }
}

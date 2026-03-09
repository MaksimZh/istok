// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "winapi/window_close.hpp"

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

TEST_CASE("Window - close", "[unit][winapi]") {
    MockWinAPI winapi;
    ECS::ECSManager ecs;

    const ECS::Entity deleted = ecs.createEntity();
    ecs.deleteEntity(deleted);
    REQUIRE(setupWindowCloseHandling(winapi, ecs, deleted).error_or("") ==
        "Invalid master entity.");

    const ECS::Entity master = ecs.createEntity();
    REQUIRE(setupWindowCloseHandling(winapi, ecs, master).error_or("") ==
        "No Dispatcher found on master entity.");

    // TODO: test empty Dispatcher
    ecs.insert(master, std::make_unique<Dispatcher>(winapi, ecs));
    REQUIRE(setupWindowCloseHandling(winapi, ecs, master));

    const ECS::Entity entity = ecs.createEntity();
    const WindowMessage closeMessage{
        reinterpret_cast<HWND>(1), WM_CLOSE, NULL, NULL};
    WindowMessageHandler handler{
        [entity, dp = ecs.get<std::unique_ptr<Dispatcher>>(master).get()](
            const WindowMessage& message
        ) noexcept {
            return dp->handleMessage(entity, message);
        }
    };
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
}

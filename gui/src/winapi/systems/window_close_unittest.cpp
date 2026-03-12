// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "winapi/systems/window_close.hpp"

#include <windows.h>

#include <istok/ecs.hpp>

#include "istok/gui/base.hpp"
#include "winapi/base/dispatcher.hpp"
#include "winapi/test_utils.hpp"

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
    ECS::ECSManager ecs;
    const ECS::Entity master = ecs.createEntity();
    REQUIRE_FALSE(setupWindowCloseHandling(ecs));

    ecs.insert(master, std::unique_ptr<WinAPIDelegate>());
    REQUIRE_FALSE(setupWindowCloseHandling(ecs));

    auto winapiContainer = std::make_unique<MockWinAPI>();
    MockWinAPI& winapi = *winapiContainer;
    ecs.insert(
        master, std::unique_ptr<WinAPIDelegate>{std::move(winapiContainer)});
    REQUIRE_FALSE(setupWindowCloseHandling(ecs));

    ecs.insert(master, std::unique_ptr<Dispatcher>());
    REQUIRE_FALSE(setupWindowCloseHandling(ecs));

    ecs.insert(master, std::make_unique<Dispatcher>(winapi));
    REQUIRE(setupWindowCloseHandling(ecs));

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

// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "winapi/units/window_size.hpp"

#include <windows.h>

#include <istok/ecs.hpp>

#include "istok/gui/base.hpp"
#include "winapi/base/dispatcher.hpp"
#include "winapi/test_utils.hpp"

using namespace Istok;
using namespace Istok::GUI;
using namespace Istok::GUI::WinAPI;

namespace {

struct MockCall {
    MAKE_MOCK0(call, void(), noexcept);
};

}  // namespace

TEST_CASE("Window - size", "[unit][winapi]") {
    ECS::ECSManager ecs;
    const ECS::Entity master = ecs.createEntity();
    MockWinAPI& winapi = setupMockWinAPI(ecs, master);
    ecs.insert(master, std::make_unique<Dispatcher>(winapi));
    REQUIRE(setupWindowSize(ecs));

    const ECS::Entity entity = ecs.createEntity();
    const WindowMessage sizeMessage{
        reinterpret_cast<HWND>(1), WM_SIZE,
        SIZE_MAXIMIZED, MAKELPARAM(5, 7)};
    WindowMessageHandler handler{
        [entity, dp = ecs.get<std::unique_ptr<Dispatcher>>(master).get()](
            const WindowMessage& message
        ) noexcept {
            return dp->handleMessage(entity, message);
        }
    };
    MockCall iteration;
    ecs.addLoopSystem([&iteration](ECS::ECSManager& ecs) noexcept {
        iteration.call(); });
    {
        REQUIRE_CALL(winapi, defWindowProc(sizeMessage)).RETURN(42);
        REQUIRE_CALL(iteration, call());
        REQUIRE(handler(sizeMessage) == 42);
    }

    ecs.insert(entity, NewWindowMarker{});
    {
        REQUIRE_CALL(winapi, defWindowProc(sizeMessage)).RETURN(42);
        FORBID_CALL(iteration, call());
        REQUIRE(handler(sizeMessage) == 42);
    }
}

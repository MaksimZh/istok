// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "winapi/units/messages.hpp"

#include <windows.h>

#include <istok/ecs.hpp>

#include "winapi/base/delegate.hpp"
#include "winapi/base/dispatcher.hpp"
#include "winapi/test_utils.hpp"

using namespace Istok;
using namespace Istok::GUI;
using namespace Istok::GUI::WinAPI;
using trompeloeil::_;


TEST_CASE("Messages", "[unit][winapi]") {
    ECS::ECSManager ecs;
    const ECS::Entity master = ecs.createEntity();
    REQUIRE_FALSE(setupMessages(ecs));

    ecs.insert(master, std::unique_ptr<WinAPIDelegate>());
    REQUIRE_FALSE(setupMessages(ecs));

    auto winapiContainer = std::make_unique<MockWinAPI>();
    MockWinAPI& winapi = *winapiContainer;
    ecs.insert(
        master, std::unique_ptr<WinAPIDelegate>{std::move(winapiContainer)});
    ecs.insert(master, std::make_unique<Dispatcher>(winapi));
    REQUIRE(setupMessages(ecs));

    REQUIRE(ecs.has<std::unique_ptr<Dispatcher>>(master));
    REQUIRE(ecs.get<std::unique_ptr<Dispatcher>>(master));
    {
        REQUIRE_CALL(winapi, getMessage(_))
            .SIDE_EFFECT(_1.message = WM_SIZE);
        REQUIRE_CALL(winapi, dispatchMessage(_))
            .WITH(_1.message == WM_SIZE);
        ecs.iterate();
    }
}

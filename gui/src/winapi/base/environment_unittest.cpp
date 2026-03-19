// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "winapi/base/environment.hpp"

#include <istok/ecs.hpp>

#include "winapi/base/dispatcher.hpp"
#include "winapi/base/null_winapi_delegate.hpp"
#include "winapi/base/winapi_delegate.hpp"

using namespace Istok;
using namespace Istok::GUI;
using namespace Istok::GUI::WinAPI;
using trompeloeil::_;


namespace {

struct Mock {
    MAKE_MOCK3(runMW, bool(ECS::ECSManager&, ECS::Entity, WinAPIDelegate&));
    MAKE_MOCK2(runW, bool(ECS::ECSManager&, WinAPIDelegate&));
    MAKE_MOCK4(runMWD, bool(
        ECS::ECSManager&, ECS::Entity, WinAPIDelegate&, Dispatcher&));
    MAKE_MOCK3(runWD, bool(ECS::ECSManager&, WinAPIDelegate&, Dispatcher&));
};

}  // namespace

TEST_CASE("Environment - runners", "[unit][winapi]") {
    Mock mock;
    auto runMW =
        [&mock](
            ECS::ECSManager& ecs, ECS::Entity master, WinAPIDelegate& winapi
        ) { return mock.runMW(ecs, master, winapi); };
    auto runW =
        [&mock](
            ECS::ECSManager& ecs, WinAPIDelegate& winapi
        ) { return mock.runW(ecs, winapi); };
    auto runMWD =
        [&mock](
            ECS::ECSManager& ecs, ECS::Entity master, WinAPIDelegate& winapi,
            Dispatcher& dispatcher
        ) { return mock.runMWD(ecs, master, winapi, dispatcher); };
    auto runWD =
        [&mock](
            ECS::ECSManager& ecs, WinAPIDelegate& winapi,
            Dispatcher& dispatcher
        ) { return mock.runWD(ecs, winapi, dispatcher); };

    ECS::ECSManager ecs;
    {
        FORBID_CALL(mock, runMW(_, _, _));
        REQUIRE_FALSE(runInEnvironment(ecs, runMW));

        FORBID_CALL(mock, runW(_, _));
        REQUIRE_FALSE(runInEnvironment(ecs, runW));

        FORBID_CALL(mock, runMWD(_, _, _, _));
        REQUIRE_FALSE(runInEnvironment(ecs, runMWD));

        FORBID_CALL(mock, runWD(_, _, _));
        REQUIRE_FALSE(runInEnvironment(ecs, runWD));
    }

    const ECS::Entity master = ecs.createEntity();
    ecs.insert(master, std::unique_ptr<WinAPIDelegate>());
    {
        FORBID_CALL(mock, runMW(_, _, _));
        REQUIRE_FALSE(runInEnvironment(ecs, runMW));

        FORBID_CALL(mock, runW(_, _));
        REQUIRE_FALSE(runInEnvironment(ecs, runW));

        FORBID_CALL(mock, runMWD(_, _, _, _));
        REQUIRE_FALSE(runInEnvironment(ecs, runMWD));

        FORBID_CALL(mock, runWD(_, _, _));
        REQUIRE_FALSE(runInEnvironment(ecs, runWD));
    }

    auto winapiContainer = std::make_unique<NullWinAPIDelegate>();
    WinAPIDelegate& winapi = *winapiContainer;
    ecs.insert(
        master, std::unique_ptr<WinAPIDelegate>{std::move(winapiContainer)});
    {
        REQUIRE_CALL(mock, runMW(_, master, _))
            .RETURN(false)
            .LR_WITH(&_1 == &ecs && &_3 == &winapi);
        REQUIRE_FALSE(runInEnvironment(ecs, runMW));

        REQUIRE_CALL(mock, runW(_, _))
            .RETURN(false)
            .LR_WITH(&_1 == &ecs && &_2 == &winapi);
        REQUIRE_FALSE(runInEnvironment(ecs, runW));
    }
    {
        REQUIRE_CALL(mock, runMW(_, master, _))
            .RETURN(true)
            .LR_WITH(&_1 == &ecs && &_3 == &winapi);
        REQUIRE(runInEnvironment(ecs, runMW));

        REQUIRE_CALL(mock, runW(_, _))
            .RETURN(true)
            .LR_WITH(&_1 == &ecs && &_2 == &winapi);
        REQUIRE(runInEnvironment(ecs, runW));

        FORBID_CALL(mock, runMWD(_, _, _, _));
        REQUIRE_FALSE(runInEnvironment(ecs, runMWD));

        FORBID_CALL(mock, runWD(_, _, _));
        REQUIRE_FALSE(runInEnvironment(ecs, runWD));
    }

    ecs.insert(master, std::unique_ptr<Dispatcher>());
    {
        FORBID_CALL(mock, runMWD(_, _, _, _));
        REQUIRE_FALSE(runInEnvironment(ecs, runMWD));

        FORBID_CALL(mock, runWD(_, _, _));
        REQUIRE_FALSE(runInEnvironment(ecs, runWD));
    }

    auto dispatcherContainer = std::make_unique<Dispatcher>(winapi);
    Dispatcher& dispatcher = *dispatcherContainer;
    ecs.insert(master, std::move(dispatcherContainer));
    {
        REQUIRE_CALL(mock, runMWD(_, master, _, _))
            .RETURN(false)
            .LR_WITH(&_1 == &ecs && &_3 == &winapi && &_4 == &dispatcher);
        REQUIRE_FALSE(runInEnvironment(ecs, runMWD));

        REQUIRE_CALL(mock, runWD(_, _, _))
            .RETURN(false)
            .LR_WITH(&_1 == &ecs && &_2 == &winapi && &_3 == &dispatcher);
        REQUIRE_FALSE(runInEnvironment(ecs, runWD));
    }
    {
        REQUIRE_CALL(mock, runMWD(_, master, _, _))
            .RETURN(true)
            .LR_WITH(&_1 == &ecs && &_3 == &winapi && &_4 == &dispatcher);
        REQUIRE(runInEnvironment(ecs, runMWD));

        REQUIRE_CALL(mock, runWD(_, _, _))
            .RETURN(true)
            .LR_WITH(&_1 == &ecs && &_2 == &winapi && &_3 == &dispatcher);
        REQUIRE(runInEnvironment(ecs, runWD));
    }
}

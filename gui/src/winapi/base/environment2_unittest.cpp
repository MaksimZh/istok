// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#define NOMINMAX
#include "winapi/base/environment2.hpp"

#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include <windows.h>

#include <istok/ecs.hpp>

#include "winapi/base/dispatcher2.hpp"
#include "winapi/base/test_utils.hpp"

using namespace Istok;
using namespace Istok::GUI;
using namespace Istok::GUI::WinAPI;
using trompeloeil::_;


namespace {

struct Mock {
    MAKE_MOCK3(runMD, bool(ECS::ECSManager&, ECS::Entity, Dispatcher2&));
    MAKE_MOCK2(runD, bool(ECS::ECSManager&, Dispatcher2&));
};

}  // namespace


TEST_CASE("Environment2 - runners", "[unit][winapi]") {
    Mock mock;
    auto runMD =
        [&mock](
            ECS::ECSManager& ecs, ECS::Entity master, Dispatcher2& dispatcher
        ) { return mock.runMD(ecs, master, dispatcher); };
    auto runD =
        [&mock](
            ECS::ECSManager& ecs, Dispatcher2& dispatcher
        ) { return mock.runD(ecs, dispatcher); };

    ECS::ECSManager ecs;
    {
        FORBID_CALL(mock, runMD(_, _, _));
        REQUIRE_FALSE(runInEnvironment(ecs, runMD));

        FORBID_CALL(mock, runD(_, _));
        REQUIRE_FALSE(runInEnvironment(ecs, runD));
    }

    const ECS::Entity master = ecs.createEntity();
    ecs.insert(master, std::unique_ptr<Dispatcher2>());
    {
        FORBID_CALL(mock, runMD(_, _, _));
        REQUIRE_FALSE(runInEnvironment(ecs, runMD));

        FORBID_CALL(mock, runD(_, _));
        REQUIRE_FALSE(runInEnvironment(ecs, runD));
    }

    auto& dispatcher = setupUnique<Dispatcher2>(ecs, master, [](
        ECS::Entity, const WindowMessage&) noexcept -> LRESULT { return 0; });
    {
        REQUIRE_CALL(mock, runMD(_, master, _))
            .RETURN(false)
            .LR_WITH(&_1 == &ecs && &_3 == &dispatcher);
        REQUIRE_FALSE(runInEnvironment(ecs, runMD));

        REQUIRE_CALL(mock, runD(_, _))
            .RETURN(false)
            .LR_WITH(&_1 == &ecs && &_2 == &dispatcher);
        REQUIRE_FALSE(runInEnvironment(ecs, runD));
    }
    {
        REQUIRE_CALL(mock, runMD(_, master, _))
            .RETURN(true)
            .LR_WITH(&_1 == &ecs && &_3 == &dispatcher);
        REQUIRE(runInEnvironment(ecs, runMD));

        REQUIRE_CALL(mock, runD(_, _))
            .RETURN(true)
            .LR_WITH(&_1 == &ecs && &_2 == &dispatcher);
        REQUIRE(runInEnvironment(ecs, runD));
    }
}
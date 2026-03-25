// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "istok/ecs/system.hpp"

#include <catch.hpp>
#include <catch2/trompeloeil.hpp>

using namespace Istok::ECS;
using namespace Istok::ECS::Internal;


namespace {

struct DeathWatch {
    Closure callback_;
    DeathWatch(Closure&& callback) : callback_(std::move(callback)) {}
    ~DeathWatch() { callback_(); }
};

struct MockClosure {
    MAKE_MOCK0(run, void(), noexcept);
    MAKE_MOCK0(kill, void(), noexcept);

    Closure get() {
        auto dw = std::make_unique<DeathWatch>([this]() noexcept { kill(); });
        return [this, x=std::move(dw)]() noexcept { run(); };
    }
};

};


TEST_CASE("System - loop", "[unit][ecs]") {
    MockClosure c1;
    MockClosure c2;
    MockClosure c3;

    auto loop = std::make_unique<ClosureLoop>();
    loop->add(c1.get());
    loop->add(c2.get());
    loop->add(c3.get());

    for (size_t i = 0; i < 3; ++ i) {
        trompeloeil::sequence seq;
        REQUIRE_CALL(c1, run()).IN_SEQUENCE(seq);
        REQUIRE_CALL(c2, run()).IN_SEQUENCE(seq)
            .LR_SIDE_EFFECT([&]() {
                // Nested iteration is forbidden.
                FORBID_CALL(c1, run());
                FORBID_CALL(c2, run());
                FORBID_CALL(c3, run());
                loop->iterate();
            });
        REQUIRE_CALL(c3, run()).IN_SEQUENCE(seq);
        loop->iterate();
    }

    {
        // Pass only possible inside iteration.
        FORBID_CALL(c1, run());
        FORBID_CALL(c2, run());
        FORBID_CALL(c3, run());
        loop->pass();
    }

    {
        trompeloeil::sequence seq;
        REQUIRE_CALL(c1, run()).IN_SEQUENCE(seq)
            .LR_SIDE_EFFECT([&]() {
                trompeloeil::sequence seq1;
                REQUIRE_CALL(c2, run())
                    .IN_SEQUENCE(seq1)
                    .LR_SIDE_EFFECT([&]() {
                        // Nested pass is forbidden.
                        FORBID_CALL(c1, run());
                        FORBID_CALL(c2, run());
                        FORBID_CALL(c3, run());
                        loop->pass();
                    });
                REQUIRE_CALL(c3, run())
                    .IN_SEQUENCE(seq1);
                loop->pass();
            });
        REQUIRE_CALL(c2, run()).IN_SEQUENCE(seq);
        REQUIRE_CALL(c3, run()).IN_SEQUENCE(seq);
        loop->iterate();
    }

    {
        trompeloeil::sequence seq;
        REQUIRE_CALL(c1, run()).IN_SEQUENCE(seq);
        REQUIRE_CALL(c2, run()).IN_SEQUENCE(seq)
            .LR_SIDE_EFFECT([&]() {
                trompeloeil::sequence seq1;
                REQUIRE_CALL(c3, run())
                    .IN_SEQUENCE(seq1)
                    .LR_SIDE_EFFECT([&]() {
                        // Iteration in pass is forbidden.
                        FORBID_CALL(c1, run());
                        FORBID_CALL(c2, run());
                        FORBID_CALL(c3, run());
                        loop->iterate();
                    });
                REQUIRE_CALL(c1, run())
                    .IN_SEQUENCE(seq1);
                loop->pass();
            });
        REQUIRE_CALL(c3, run()).IN_SEQUENCE(seq);
        loop->iterate();
    }

    {
        trompeloeil::sequence seq;
        REQUIRE_CALL(c1, run()).IN_SEQUENCE(seq);
        REQUIRE_CALL(c2, run()).IN_SEQUENCE(seq);
        REQUIRE_CALL(c3, run()).IN_SEQUENCE(seq)
            .LR_SIDE_EFFECT([&]() {
                trompeloeil::sequence seq1;
                REQUIRE_CALL(c1, run())
                    .IN_SEQUENCE(seq1);
                REQUIRE_CALL(c2, run())
                    .IN_SEQUENCE(seq1);
                loop->pass();
            });
        loop->iterate();
    }

    {
        trompeloeil::sequence seq;
        REQUIRE_CALL(c3, kill()).IN_SEQUENCE(seq);
        REQUIRE_CALL(c2, kill()).IN_SEQUENCE(seq);
        REQUIRE_CALL(c1, kill()).IN_SEQUENCE(seq);
        loop.reset();
    }
}

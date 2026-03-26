// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "istok/ecs/system.hpp"

#include <catch.hpp>
#include <catch2/trompeloeil.hpp>

#include "test_utils.hpp"

using namespace Istok::ECS;
using namespace Istok::ECS::Internal;


TEST_CASE("System - ClosureLoop", "[unit][ecs]") {
    MockClosure c1;
    MockClosure c2;
    MockClosure c3;

    ClosureLoop loop;
    loop.add(c1.get());
    loop.add(c2.get());
    loop.add(c3.get());

    for (size_t i = 0; i < 3; ++ i) {
        trompeloeil::sequence seq;
        REQUIRE_CALL(c1, run()).IN_SEQUENCE(seq);
        REQUIRE_CALL(c2, run()).IN_SEQUENCE(seq)
            .LR_SIDE_EFFECT([&]() {
                // Nested iteration is forbidden.
                FORBID_CALL(c1, run());
                FORBID_CALL(c2, run());
                FORBID_CALL(c3, run());
                loop.iterate();
            });
        REQUIRE_CALL(c3, run()).IN_SEQUENCE(seq);
        loop.iterate();
    }

    {
        // Pass only possible inside iteration.
        FORBID_CALL(c1, run());
        FORBID_CALL(c2, run());
        FORBID_CALL(c3, run());
        loop.pass();
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
                        loop.pass();
                    });
                REQUIRE_CALL(c3, run())
                    .IN_SEQUENCE(seq1);
                loop.pass();
            });
        REQUIRE_CALL(c2, run()).IN_SEQUENCE(seq);
        REQUIRE_CALL(c3, run()).IN_SEQUENCE(seq);
        loop.iterate();
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
                        loop.iterate();
                    });
                REQUIRE_CALL(c1, run())
                    .IN_SEQUENCE(seq1);
                loop.pass();
            });
        REQUIRE_CALL(c3, run()).IN_SEQUENCE(seq);
        loop.iterate();
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
                loop.pass();
            });
        loop.iterate();
    }

    {
        trompeloeil::sequence seq;
        REQUIRE_CALL(c3, kill()).IN_SEQUENCE(seq);
        REQUIRE_CALL(c2, kill()).IN_SEQUENCE(seq);
        REQUIRE_CALL(c1, kill()).IN_SEQUENCE(seq);
        loop.clear();
    }
}


TEST_CASE("System - ClosureQueue", "[unit][ecs]") {
    MockClosure c1;
    MockClosure c2;
    MockClosure c3;

    ClosureQueue cq;
    cq.add(c1.get());
    cq.add(c2.get());
    cq.add(c3.get());

    {
        trompeloeil::sequence seq;
        REQUIRE_CALL(c1, run()).IN_SEQUENCE(seq);
        REQUIRE_CALL(c1, kill()).IN_SEQUENCE(seq);
        REQUIRE_CALL(c2, run()).IN_SEQUENCE(seq);
        REQUIRE_CALL(c2, kill()).IN_SEQUENCE(seq);
        REQUIRE_CALL(c3, run()).IN_SEQUENCE(seq);
        REQUIRE_CALL(c3, kill()).IN_SEQUENCE(seq);
        cq.launch();
    }
}


TEST_CASE("System - ClosureStack", "[unit][ecs]") {
    MockClosure c1;
    MockClosure c2;
    MockClosure c3;

    ClosureStack cs;
    cs.add(c1.get());
    cs.add(c2.get());
    cs.add(c3.get());

    {
        trompeloeil::sequence seq;
        REQUIRE_CALL(c3, run()).IN_SEQUENCE(seq);
        REQUIRE_CALL(c3, kill()).IN_SEQUENCE(seq);
        REQUIRE_CALL(c2, run()).IN_SEQUENCE(seq);
        REQUIRE_CALL(c2, kill()).IN_SEQUENCE(seq);
        REQUIRE_CALL(c1, run()).IN_SEQUENCE(seq);
        REQUIRE_CALL(c1, kill()).IN_SEQUENCE(seq);
        cs.launch();
    }
}


TEST_CASE("System - manager", "[unit][ecs]") {
    MockClosure loop1;
    MockClosure loop2;
    MockClosure head1;
    MockClosure head2;
    MockClosure tail1;
    MockClosure tail2;
    MockValue comp;

    SystemManager sm;
    sm.addLoop(loop1.get());
    sm.addLoop(loop2.get());
    sm.addHead(head1.get());
    sm.addHead(head2.get());
    sm.addTail(tail1.get());
    sm.addTail(tail2.get());

    {
        trompeloeil::sequence seq;
        REQUIRE_CALL(loop1, run()).IN_SEQUENCE(seq);
        REQUIRE_CALL(loop2, run()).IN_SEQUENCE(seq)
            .LR_SIDE_EFFECT([&]() {
                FORBID_CALL(loop1, run());
                FORBID_CALL(loop2, run());
                sm.iterate();
            });
        sm.iterate();
    }

    {
        FORBID_CALL(loop1, run());
        FORBID_CALL(loop2, run());
        sm.pass();
    }

    {
        trompeloeil::sequence seq;
        REQUIRE_CALL(loop1, run()).IN_SEQUENCE(seq)
            .LR_SIDE_EFFECT([&]() {
                trompeloeil::sequence seq1;
                REQUIRE_CALL(loop2, run())
                    .IN_SEQUENCE(seq1)
                    .LR_SIDE_EFFECT([&]() {
                        FORBID_CALL(loop1, run());
                        FORBID_CALL(loop2, run());
                        sm.pass();
                    });
                sm.pass();
            });
        REQUIRE_CALL(loop2, run()).IN_SEQUENCE(seq);
        sm.iterate();
    }

    {
        trompeloeil::sequence seq;
        REQUIRE_CALL(loop2, kill()).IN_SEQUENCE(seq);
        REQUIRE_CALL(loop1, kill()).IN_SEQUENCE(seq);
        REQUIRE_CALL(head1, run()).IN_SEQUENCE(seq);
        REQUIRE_CALL(head1, kill()).IN_SEQUENCE(seq);
        REQUIRE_CALL(head2, run()).IN_SEQUENCE(seq);
        REQUIRE_CALL(head2, kill()).IN_SEQUENCE(seq);
        REQUIRE_CALL(tail2, run()).IN_SEQUENCE(seq);
        REQUIRE_CALL(tail2, kill()).IN_SEQUENCE(seq);
        REQUIRE_CALL(tail1, run()).IN_SEQUENCE(seq);
        REQUIRE_CALL(tail1, kill()).IN_SEQUENCE(seq);
        sm.clear();
    }
}

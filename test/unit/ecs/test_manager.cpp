// test_manager.cpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <ecs/manager.hpp>

using namespace Istok::ECS;


namespace {
    struct A {
        int value;
        bool operator==(const A&) const = default;
    };

    struct B {
        int value;
        bool operator==(const B&) const = default;
    };

    struct C {
        int value;
        bool operator==(const C&) const = default;
    };
}


TEST_CASE("ECS - manager", "[unit][ecs]") {
}

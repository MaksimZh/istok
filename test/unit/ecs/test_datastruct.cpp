// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <ecs/datastruct.hpp>

namespace ecs = Istok::ECS;

namespace {
    struct Obj {
        int value;

        bool operator==(const Obj&) const = default;

        struct Hasher {
            size_t operator()(const Obj& obj) const {
                return std::hash<int>()(obj.value);
            }
        };
    };
}


TEST_CASE("ECS Data Structures - queue", "[unit][ecs]") {
    ecs::Queue<Obj> q;
    REQUIRE(q.empty() == true);
    q.push(Obj(1));
    REQUIRE(q.empty() == false);
    REQUIRE(q.pop() == Obj(1));
    REQUIRE(q.empty() == true);
    q.push(Obj(2));
    q.push(Obj(3));
    REQUIRE(q.empty() == false);
    REQUIRE(q.pop() == Obj(2));
    REQUIRE(q.empty() == false);
    REQUIRE(q.pop() == Obj(3));
    REQUIRE(q.empty() == true);
}

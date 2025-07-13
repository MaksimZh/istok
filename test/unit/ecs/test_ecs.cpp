// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <ecs/ecs.hpp>

#include <ranges>
#include <vector>


namespace {
    Entity fakeEntity(size_t index) {
        return Entity(EntityIndex(index), EntityGeneration(0));
    }

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


TEST_CASE("ECS - component manager", "[unit][ecs]") {
    ComponentManager manager;
    Entity e0 = fakeEntity(0);
    Entity e1 = fakeEntity(1);
    
    // adding and checking components
    REQUIRE(manager.has<A>(e0) == false);
    REQUIRE(manager.has<B>(e0) == false);
    REQUIRE(manager.has<C>(e0) == false);
    manager.add(e0, A{0});
    REQUIRE(manager.has<A>(e0) == true);
    REQUIRE(manager.has<B>(e0) == false);
    REQUIRE(manager.has<C>(e0) == false);
    manager.add(e0, B{0});
    manager.add(e1, B{1});
    manager.add(e1, C{1});
    REQUIRE(manager.has<A>(e0) == true);
    REQUIRE(manager.has<B>(e0) == true);
    REQUIRE(manager.has<C>(e0) == false);
    REQUIRE(manager.has<A>(e1) == false);
    REQUIRE(manager.has<B>(e1) == true);
    REQUIRE(manager.has<C>(e1) == true);

    // reading components
    REQUIRE(manager.get<A>(e0) == A{0});
    REQUIRE(manager.get<B>(e0) == B{0});
    REQUIRE(manager.get<B>(e1) == B{1});
    REQUIRE(manager.get<C>(e1) == C{1});

    // removing components
    manager.remove<A>(e0);
    manager.remove<C>(e1);
    REQUIRE(manager.has<A>(e0) == false);
    REQUIRE(manager.has<B>(e0) == true);
    REQUIRE(manager.has<C>(e0) == false);
    REQUIRE(manager.has<A>(e1) == false);
    REQUIRE(manager.has<B>(e1) == true);
    REQUIRE(manager.has<C>(e1) == false);
}


TEST_CASE("ECS - component manager clean", "[unit][ecs]") {
    ComponentManager manager;
    Entity e = fakeEntity(0);
    manager.add(e, A{0});
    manager.add(e, B{0});
    manager.add(e, C{0});
    REQUIRE(manager.has<A>(e) == true);
    REQUIRE(manager.has<B>(e) == true);
    REQUIRE(manager.has<C>(e) == true);
    manager.clean(e);
    REQUIRE(manager.has<A>(e) == false);
    REQUIRE(manager.has<B>(e) == false);
    REQUIRE(manager.has<C>(e) == false);
}


TEST_CASE("ECS - component manager view", "[unit][ecs]") {
    ComponentManager manager;
    Entity a = fakeEntity(0);
    Entity b = fakeEntity(1);
    Entity c = fakeEntity(2);
    Entity ab = fakeEntity(3);
    Entity bc = fakeEntity(4);
    Entity ca = fakeEntity(5);
    Entity abc = fakeEntity(6);
    manager.add(a, A{0});
    manager.add(b, B{1});
    manager.add(c, C{2});
    manager.add(ab, A{3});
    manager.add(ab, B{3});
    manager.add(bc, B{4});
    manager.add(bc, C{4});
    manager.add(ca, C{5});
    manager.add(ca, A{5});
    manager.add(abc, A{6});
    manager.add(abc, B{6});
    manager.add(abc, C{6});
    auto v = manager.getView<A>();
    //REQUIRE(std::ranges::equal(manager.getView<A>(), std::vector{a, ab, ca, abc}));
}

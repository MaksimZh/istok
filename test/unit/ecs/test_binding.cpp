// test_component.cpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include "helper.hpp"

#include <ecs/binding.hpp>

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


TEST_CASE("ECS - entity binding", "[unit][ecs]") {
    EntityComponentManager manager;
    Entity entity = manager.createEntity();
    BoundEntity bound = BoundEntity(manager, entity);
    REQUIRE(&bound.getManager() == &manager);
    REQUIRE(bound.getEntity() == entity);
    REQUIRE(bound.isValid() == true);
    REQUIRE(bound.has<A>() == false);
    REQUIRE(bound.has<B>() == false);
    REQUIRE(bound.has<C>() == false);

    SECTION("destroy") {
        bound.destroy();
        REQUIRE(bound.isValid() == false);
    }

    SECTION("set components") {
        bound.set<A>(A{0});
        REQUIRE(bound.has<A>() == true);
        REQUIRE(bound.has<B>() == false);
        REQUIRE(bound.has<C>() == false);
        REQUIRE(bound.get<A>() == A{0});

        bound.set<B>(B{1});
        REQUIRE(bound.has<A>() == true);
        REQUIRE(bound.has<B>() == true);
        REQUIRE(bound.has<C>() == false);
        REQUIRE(bound.get<A>() == A{0});
        REQUIRE(bound.get<B>() == B{1});

        SECTION("change component") {
            bound.get<A>().value = 42;
            REQUIRE(bound.get<A>() == A{42});
            REQUIRE(bound.get<B>() == B{1});
        }

        SECTION("remove components") {
            bound.remove<A>();
            REQUIRE(bound.has<A>() == false);
            REQUIRE(bound.has<B>() == true);
            REQUIRE(bound.has<C>() == false);
            bound.remove<B>();
            REQUIRE(bound.has<A>() == false);
            REQUIRE(bound.has<B>() == false);
            REQUIRE(bound.has<C>() == false);
        }

        SECTION("destroy") {
            bound.destroy();
            REQUIRE(bound.isValid() == false);
        }
    }
};

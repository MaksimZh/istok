// test_manager.cpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include "helper.hpp"

#include <ecs/manager.hpp>

using namespace Istok::ECS;

#include <ranges>
#include <set>

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
    EntityComponentManager manager(3);
    REQUIRE(isSameEntitySet(manager.view<A>(), EntityUSet{}));

    SECTION("remove all") {
        manager.removeAll<A>();
        REQUIRE(isSameEntitySet(manager.view<A>(), EntityUSet{}));
    }

    SECTION("clear") {
        manager.clear();
        REQUIRE(isSameEntitySet(manager.view<A>(), EntityUSet{}));
    }

    SECTION("single entity") {
        Entity e0 = manager.createEntity();
        REQUIRE(manager.isValidEntity(e0) == true);
        REQUIRE(manager.has<A>(e0) == false);
        REQUIRE(manager.has<B>(e0) == false);
        REQUIRE(manager.has<C>(e0) == false);
        REQUIRE(isSameEntitySet(manager.view<A>(), EntityUSet{}));

        manager.set(e0, A{0});
        REQUIRE(manager.has<A>(e0) == true);
        REQUIRE(manager.has<B>(e0) == false);
        REQUIRE(manager.has<C>(e0) == false);
        REQUIRE(isSameEntitySet(manager.view<A>(), EntityUSet{e0}));
        REQUIRE(isSameEntitySet(manager.view<A, B>(), EntityUSet{}));
        REQUIRE(manager.get<A>(e0) == A{0});

        manager.set(e0, B{0});
        REQUIRE(manager.has<A>(e0) == true);
        REQUIRE(manager.has<B>(e0) == true);
        REQUIRE(manager.has<C>(e0) == false);
        REQUIRE(isSameEntitySet(manager.view<A, B>(), EntityUSet{e0}));
        REQUIRE(manager.get<A>(e0) == A{0});
        REQUIRE(manager.get<B>(e0) == B{0});

        SECTION("change component") {
            manager.get<A>(e0).value = 42;
            REQUIRE(manager.get<A>(e0) == A{42});
            REQUIRE(manager.get<B>(e0) == B{0});
        }

        SECTION("remove components") {
            manager.remove<A>(e0);
            REQUIRE(manager.has<A>(e0) == false);
            REQUIRE(manager.has<B>(e0) == true);
            REQUIRE(manager.has<C>(e0) == false);
            REQUIRE(isSameEntitySet(manager.view<A>(), EntityUSet{}));
            REQUIRE(isSameEntitySet(manager.view<B>(), EntityUSet{e0}));
            manager.remove<B>(e0);
            REQUIRE(manager.has<A>(e0) == false);
            REQUIRE(manager.has<B>(e0) == false);
            REQUIRE(manager.has<C>(e0) == false);
            REQUIRE(isSameEntitySet(manager.view<A>(), EntityUSet{}));
            REQUIRE(isSameEntitySet(manager.view<B>(), EntityUSet{}));
        }

        SECTION("remove all") {
            manager.removeAll<A>();
            REQUIRE(isSameEntitySet(manager.view<A>(), EntityUSet{}));
        }

        SECTION("clear") {
            manager.clear();
            REQUIRE(isSameEntitySet(manager.view<A>(), EntityUSet{}));
            REQUIRE(isSameEntitySet(manager.view<B>(), EntityUSet{}));
            REQUIRE(isSameEntitySet(manager.view<C>(), EntityUSet{}));
        }

        SECTION("destroy entity") {
            manager.destroyEntity(e0);
            REQUIRE(manager.isValidEntity(e0) == false);
            REQUIRE(isSameEntitySet(manager.view<A>(), EntityUSet{}));
            REQUIRE(isSameEntitySet(manager.view<B>(), EntityUSet{}));
        }
    }

    SECTION("full storage") {
        Entity e0 = manager.createEntity();
        Entity e1 = manager.createEntity();
        Entity e2 = manager.createEntity();
        REQUIRE(manager.isValidEntity(e0) == true);
        REQUIRE(manager.isValidEntity(e1) == true);
        REQUIRE(manager.isValidEntity(e2) == true);
        REQUIRE(manager.has<A>(e0) == false);
        REQUIRE(manager.has<B>(e0) == false);
        REQUIRE(manager.has<C>(e0) == false);
        REQUIRE(manager.has<A>(e1) == false);
        REQUIRE(manager.has<B>(e1) == false);
        REQUIRE(manager.has<C>(e1) == false);
        REQUIRE(manager.has<A>(e2) == false);
        REQUIRE(manager.has<B>(e2) == false);
        REQUIRE(manager.has<C>(e2) == false);
        manager.set(e0, A{0});
        manager.set(e0, B{0});
        manager.set(e1, B{1});
        manager.set(e1, C{1});
        manager.set(e2, A{2});
        manager.set(e2, C{2});
        REQUIRE(manager.has<A>(e0) == true);
        REQUIRE(manager.has<B>(e0) == true);
        REQUIRE(manager.has<C>(e0) == false);
        REQUIRE(manager.has<A>(e1) == false);
        REQUIRE(manager.has<B>(e1) == true);
        REQUIRE(manager.has<C>(e1) == true);
        REQUIRE(manager.has<A>(e2) == true);
        REQUIRE(manager.has<B>(e2) == false);
        REQUIRE(manager.has<C>(e2) == true);
        REQUIRE(isSameEntitySet(manager.view<A>(), EntityUSet{e0, e2}));
        REQUIRE(isSameEntitySet(manager.view<B>(), EntityUSet{e0, e1}));
        REQUIRE(isSameEntitySet(manager.view<C>(), EntityUSet{e1, e2}));
        REQUIRE(isSameEntitySet(manager.view<A, B>(), EntityUSet{e0}));
        REQUIRE(isSameEntitySet(manager.view<B, C>(), EntityUSet{e1}));
        REQUIRE(isSameEntitySet(manager.view<A, C>(), EntityUSet{e2}));
        REQUIRE(isSameEntitySet(manager.view<A, B, C>(), EntityUSet{}));
        REQUIRE(manager.get<A>(e0) == A{0});
        REQUIRE(manager.get<B>(e0) == B{0});
        REQUIRE(manager.get<B>(e1) == B{1});
        REQUIRE(manager.get<C>(e1) == C{1});
        REQUIRE(manager.get<A>(e2) == A{2});
        REQUIRE(manager.get<C>(e2) == C{2});

        SECTION("change component") {
            manager.get<A>(e0).value = 42;
            manager.get<B>(e1).value = 17;
            manager.get<C>(e2).value = 39;
            REQUIRE(manager.get<A>(e0) == A{42});
            REQUIRE(manager.get<B>(e0) == B{0});
            REQUIRE(manager.get<B>(e1) == B{17});
            REQUIRE(manager.get<C>(e1) == C{1});
            REQUIRE(manager.get<A>(e2) == A{2});
            REQUIRE(manager.get<C>(e2) == C{39});
        }

        SECTION("remove components") {
            manager.remove<A>(e0);
            manager.remove<B>(e1);
            manager.remove<C>(e2);
            REQUIRE(manager.has<A>(e0) == false);
            REQUIRE(manager.has<B>(e0) == true);
            REQUIRE(manager.has<C>(e0) == false);
            REQUIRE(manager.has<A>(e1) == false);
            REQUIRE(manager.has<B>(e1) == false);
            REQUIRE(manager.has<C>(e1) == true);
            REQUIRE(manager.has<A>(e2) == true);
            REQUIRE(manager.has<B>(e2) == false);
            REQUIRE(manager.has<C>(e2) == false);
            REQUIRE(isSameEntitySet(manager.view<A>(), EntityUSet{e2}));
            REQUIRE(isSameEntitySet(manager.view<B>(), EntityUSet{e0}));
            REQUIRE(isSameEntitySet(manager.view<C>(), EntityUSet{e1}));
            REQUIRE(isSameEntitySet(manager.view<A, B>(), EntityUSet{}));
            REQUIRE(isSameEntitySet(manager.view<B, C>(), EntityUSet{}));
            REQUIRE(isSameEntitySet(manager.view<A, C>(), EntityUSet{}));
            REQUIRE(isSameEntitySet(manager.view<A, B, C>(), EntityUSet{}));
            REQUIRE(manager.get<B>(e0) == B{0});
            REQUIRE(manager.get<C>(e1) == C{1});
            REQUIRE(manager.get<A>(e2) == A{2});
        }

        SECTION("remove all") {
            manager.removeAll<A>();
            REQUIRE(isSameEntitySet(manager.view<A>(), EntityUSet{}));
        }

        SECTION("clear") {
            manager.clear();
            REQUIRE(isSameEntitySet(manager.view<A>(), EntityUSet{}));
            REQUIRE(isSameEntitySet(manager.view<B>(), EntityUSet{}));
            REQUIRE(isSameEntitySet(manager.view<C>(), EntityUSet{}));
        }

        SECTION("destroy entity") {
            manager.destroyEntity(e0);
            manager.destroyEntity(e1);
            REQUIRE(manager.isValidEntity(e0) == false);
            REQUIRE(manager.isValidEntity(e1) == false);
            REQUIRE(manager.isValidEntity(e2) == true);
        }
    }

    SECTION("many entities") {
        EntityUSet entities;
        // create much more entities than initial capacity
        for (int i = 0; i < 20; i++) {
            entities.insert(manager.createEntity());
        }
        REQUIRE(entities.size() == 20);
        for (auto& e : entities) { REQUIRE(manager.isValidEntity(e) == true); }

        SECTION("destroy") {
            EntityUSet old;
            for (int i = 0; i < 20; i++) {
                Entity e = *entities.begin();
                manager.destroyEntity(e);
                entities.erase(e);
                old.insert(e);
            }

            for (auto e : old) { REQUIRE(manager.isValidEntity(e) == false); }
            for (auto e : entities) { REQUIRE(manager.isValidEntity(e) == true); }

            for (int i = 0; i < 20; i++) {
                entities.insert(manager.createEntity());
            }

            for (auto e : old) { REQUIRE(manager.isValidEntity(e) == false); }
            for (auto e : entities) { REQUIRE(manager.isValidEntity(e) == true); }
        }
    }

    SECTION("all ABC combinations") {
        Entity a = manager.createEntity();
        manager.set(a, A{0});
        Entity b = manager.createEntity();
        manager.set(b, B{1});
        Entity c = manager.createEntity();
        manager.set(c, C{2});
        Entity ab = manager.createEntity();
        manager.set(ab, A{3});
        manager.set(ab, B{4});
        Entity ac = manager.createEntity();
        manager.set(ac, A{5});
        manager.set(ac, C{6});
        Entity bc = manager.createEntity();
        manager.set(bc, B{7});
        manager.set(bc, C{8});
        Entity abc = manager.createEntity();
        manager.set(abc, A{9});
        manager.set(abc, B{10});
        manager.set(abc, C{11});
        REQUIRE(isSameEntitySet(manager.view<A>(), EntityUSet{a, ab, ac, abc}));
        REQUIRE(isSameEntitySet(manager.view<B>(), EntityUSet{b, ab, bc, abc}));
        REQUIRE(isSameEntitySet(manager.view<C>(), EntityUSet{c, ac, bc, abc}));
        REQUIRE(isSameEntitySet(manager.view<A, B>(), EntityUSet{ab, abc}));
        REQUIRE(isSameEntitySet(manager.view<B, C>(), EntityUSet{bc, abc}));
        REQUIRE(isSameEntitySet(manager.view<A, C>(), EntityUSet{ac, abc}));
        REQUIRE(isSameEntitySet(manager.view<A, B, C>(), EntityUSet{abc}));
        REQUIRE(isSameEntitySet(
            manager.view<A>().exclude<B>(),
            EntityUSet{a, ac}));
    }
}

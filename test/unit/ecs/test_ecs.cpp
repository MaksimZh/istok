// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <ecs/ecs.hpp>

using namespace Istok::ECS;

#include <ranges>
#include <unordered_set>


namespace {
    Entity fakeEntity(size_t index) {
        return Entity(index, 0);
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

    using EntityUSet = std::unordered_set<Entity, Entity::Hasher>;

    template <std::ranges::input_range R>
    bool isSameEntitySet(const R& x, const EntityUSet& y) {
        EntityUSet xs(std::ranges::begin(x), std::ranges::end(x));
        bool allUnique = std::ranges::size(x) == xs.size();
        return allUnique && xs == y;
    }
}


TEST_CASE("ECS - component storage", "[unit][ecs]") {
    ComponentStorageOf<A> storage;
    REQUIRE(storage.size() == 0);
    REQUIRE(isSameEntitySet(storage.byEntity(), EntityUSet{}));

    SECTION("single entity") {
        Entity e = fakeEntity(0);
        REQUIRE(storage.has(e) == false);
        storage.insert(e, A{0});
        REQUIRE(storage.has(e) == true);
        REQUIRE(storage.get(e) == A{0});
        REQUIRE(storage.size() == 1);
        REQUIRE(isSameEntitySet(storage.byEntity(), EntityUSet{e}));

        SECTION("edit") {
            storage.get(e).value = 42;
            REQUIRE(storage.get(e) == A{42});
        }

        SECTION("replace") {
            storage.insert(e, A{42});
            REQUIRE(storage.get(e) == A{42});
        }

        SECTION("remove") {
            storage.remove(e);
            REQUIRE(storage.has(e) == false);
            REQUIRE(storage.size() == 0);
            REQUIRE(isSameEntitySet(storage.byEntity(), EntityUSet{}));
        }
    }


    SECTION("many entities") {
        Entity e0 = fakeEntity(0);
        Entity e1 = fakeEntity(1);
        Entity e2 = fakeEntity(2);
        REQUIRE(storage.has(e0) == false);
        REQUIRE(storage.has(e1) == false);
        REQUIRE(storage.has(e2) == false);
        storage.insert(e0, A{0});
        storage.insert(e1, A{1});
        storage.insert(e2, A{2});
        REQUIRE(storage.has(e0) == true);
        REQUIRE(storage.has(e1) == true);
        REQUIRE(storage.has(e2) == true);
        REQUIRE(storage.get(e0) == A{0});
        REQUIRE(storage.get(e1) == A{1});
        REQUIRE(storage.get(e2) == A{2});
        REQUIRE(storage.size() == 3);
        REQUIRE(isSameEntitySet(storage.byEntity(), EntityUSet{e0, e1, e2}));

        SECTION("edit") {
            storage.get(e1).value = 42;
            REQUIRE(storage.get(e1) == A{42});
        }

        SECTION("replace") {
            storage.insert(e1, A{42});
            REQUIRE(storage.get(e1) == A{42});
        }

        SECTION("remove") {
            storage.remove(e1);
            REQUIRE(storage.has(e0) == true);
            REQUIRE(storage.has(e1) == false);
            REQUIRE(storage.has(e2) == true);
            REQUIRE(storage.get(e0) == A{0});
            REQUIRE(storage.get(e2) == A{2});
            REQUIRE(storage.size() == 2);
            REQUIRE(isSameEntitySet(storage.byEntity(), EntityUSet{e0, e2}));
            
            storage.remove(e2);
            REQUIRE(storage.has(e0) == true);
            REQUIRE(storage.has(e1) == false);
            REQUIRE(storage.has(e2) == false);
            REQUIRE(storage.get(e0) == A{0});
            REQUIRE(storage.size() == 1);
            REQUIRE(isSameEntitySet(storage.byEntity(), EntityUSet{e0}));
        }
    }
}

/*
TEST_CASE("ECS - component storage view", "[unit][ecs]") {
    static_assert(std::ranges::forward_range<ComponentStorageOf<A>::View>);
    ComponentStorageOf<A> storage;
    Entity e0 = fakeEntity(0);
    Entity e1 = fakeEntity(1);
    Entity e2 = fakeEntity(2);
    
    REQUIRE(isSameEntitySet(storage.getView(), EntityUSet{}));
    
    storage.insert(e0, A{0});
    storage.insert(e1, A{1});
    storage.insert(e2, A{2});

    REQUIRE(isSameEntitySet(storage.getView(), EntityUSet{e0, e1, e2}));

    storage.remove(e0);
    storage.remove(e2);
 
    REQUIRE(isSameEntitySet(storage.getView(), EntityUSet{e1}));
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
*/
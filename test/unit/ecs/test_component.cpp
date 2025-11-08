// test_component.cpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include "helper.hpp"

#include <ecs/component.hpp>

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

    using StorageUSet = std::unordered_set<ComponentStorage*>;

    template <std::ranges::input_range R>
    bool isSameStorageSet(const R& x, const StorageUSet& y) {
        std::vector<ComponentStorage*> xv;
        for (auto& v : x) {
            xv.push_back(&v);
        }
        StorageUSet xs(xv.begin(), xv.end());
        bool allUnique = (xv.size() == xs.size());
        return allUnique && xs == y;
    }
}


TEST_CASE("ECS - component storage", "[unit][ecs]") {
    ComponentStorageOf<A> storage;
    REQUIRE(storage.size() == 0);
    REQUIRE(isSameEntitySet(storage.byEntity(), EntityUSet{}));
    
    SECTION("remove all") {
        storage.removeAll();
        REQUIRE(storage.size() == 0);
        REQUIRE(isSameEntitySet(storage.byEntity(), EntityUSet{}));
    }

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

        SECTION("remove all") {
            storage.removeAll();
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

        SECTION("remove all") {
            storage.removeAll();
            REQUIRE(storage.size() == 0);
            REQUIRE(isSameEntitySet(storage.byEntity(), EntityUSet{}));
        }
    }
}


TEST_CASE("ECS - component storage manager", "[unit][ecs]") {
    ComponentStorageManager manager;
    REQUIRE(manager.hasStorage<A>() == false);
    REQUIRE(manager.hasStorage<B>() == false);
    REQUIRE(manager.hasStorage<C>() == false);

    SECTION("single") {
        ComponentStorageOf<A>& a = manager.getOrCreateStorage<A>();
        REQUIRE(manager.hasStorage<A>() == true);
        REQUIRE(manager.hasStorage<B>() == false);
        REQUIRE(manager.hasStorage<C>() == false);
        REQUIRE(&manager.getStorage<A>() == &a);
        REQUIRE(isSameStorageSet(manager.byStorage(), StorageUSet{&a}));
    }

    SECTION("many") {
        ComponentStorageOf<A>& a = manager.getOrCreateStorage<A>();
        ComponentStorageOf<B>& b = manager.getOrCreateStorage<B>();
        REQUIRE(manager.hasStorage<A>() == true);
        REQUIRE(manager.hasStorage<B>() == true);
        REQUIRE(manager.hasStorage<C>() == false);
        REQUIRE(&manager.getStorage<A>() == &a);
        REQUIRE(&manager.getStorage<B>() == &b);
        REQUIRE(isSameStorageSet(manager.byStorage(), StorageUSet{&a, &b}));

        ComponentStorageOf<C>& c = manager.getOrCreateStorage<C>();
        REQUIRE(manager.hasStorage<A>() == true);
        REQUIRE(manager.hasStorage<B>() == true);
        REQUIRE(manager.hasStorage<C>() == true);
        REQUIRE(&manager.getStorage<C>() == &c);
        REQUIRE(isSameStorageSet(manager.byStorage(), StorageUSet{&a, &b, &c}));
    }
}

static_assert(std::forward_iterator<EntityStorageIterator>);
static_assert(std::ranges::forward_range<EntityView>);

TEST_CASE("ECS - component manager", "[unit][ecs]") {
    ComponentManager manager;
    
    SECTION("one entity") {
        Entity e0 = fakeEntity(0);
        REQUIRE(manager.has<A>(e0) == false);
        REQUIRE(manager.has<B>(e0) == false);
        REQUIRE(manager.has<C>(e0) == false);
        manager.insert(e0, A{0});
        REQUIRE(manager.has<A>(e0) == true);
        REQUIRE(manager.has<B>(e0) == false);
        REQUIRE(manager.has<C>(e0) == false);
        REQUIRE(manager.get<A>(e0) == A{0});
        manager.insert(e0, B{0});
        REQUIRE(manager.has<A>(e0) == true);
        REQUIRE(manager.has<B>(e0) == true);
        REQUIRE(manager.has<C>(e0) == false);
        REQUIRE(manager.get<A>(e0) == A{0});
        REQUIRE(manager.get<B>(e0) == B{0});

        SECTION("remove") {
            manager.remove<A>(e0);
            REQUIRE(manager.has<A>(e0) == false);
            REQUIRE(manager.has<B>(e0) == true);
            REQUIRE(manager.has<C>(e0) == false);
            REQUIRE(manager.get<B>(e0) == B{0});
        }

        SECTION("remove all") {
            manager.removeAll<A>();
            REQUIRE(manager.has<A>(e0) == false);
            REQUIRE(manager.has<B>(e0) == true);
            REQUIRE(manager.has<C>(e0) == false);
            REQUIRE(manager.get<B>(e0) == B{0});
        }

        SECTION("clean") {
            manager.clean(e0);
            REQUIRE(manager.has<A>(e0) == false);
            REQUIRE(manager.has<B>(e0) == false);
            REQUIRE(manager.has<C>(e0) == false);
        }

        SECTION("view") {
            REQUIRE(isSameEntitySet(manager.view<A>(), EntityUSet{e0}));
            REQUIRE(isSameEntitySet(manager.view<B>(), EntityUSet{e0}));
            REQUIRE(isSameEntitySet(manager.view<C>(), EntityUSet{}));
            REQUIRE(isSameEntitySet(manager.view<A, B>(), EntityUSet{e0}));
            REQUIRE(isSameEntitySet(manager.view<A, C>(), EntityUSet{}));
        }
    }

    SECTION("many entities") {
        Entity e0 = fakeEntity(0);
        Entity e1 = fakeEntity(1);
        Entity e2 = fakeEntity(2);
        manager.insert(e0, A{0});
        manager.insert(e0, B{0});
        manager.insert(e1, B{1});
        manager.insert(e1, C{1});
        manager.insert(e2, A{2});
        manager.insert(e2, C{2});
        REQUIRE(manager.has<A>(e0) == true);
        REQUIRE(manager.has<B>(e0) == true);
        REQUIRE(manager.has<C>(e0) == false);
        REQUIRE(manager.has<A>(e1) == false);
        REQUIRE(manager.has<B>(e1) == true);
        REQUIRE(manager.has<C>(e1) == true);
        REQUIRE(manager.has<A>(e2) == true);
        REQUIRE(manager.has<B>(e2) == false);
        REQUIRE(manager.has<C>(e2) == true);
        REQUIRE(manager.get<A>(e0) == A{0});
        REQUIRE(manager.get<B>(e0) == B{0});
        REQUIRE(manager.get<B>(e1) == B{1});
        REQUIRE(manager.get<C>(e1) == C{1});
        REQUIRE(manager.get<A>(e2) == A{2});
        REQUIRE(manager.get<C>(e2) == C{2});

        SECTION("remove") {
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
            REQUIRE(manager.get<B>(e0) == B{0});
            REQUIRE(manager.get<C>(e1) == C{1});
            REQUIRE(manager.get<A>(e2) == A{2});
        }

        SECTION("removeAll") {
            manager.removeAll<A>();
            REQUIRE(manager.has<A>(e0) == false);
            REQUIRE(manager.has<B>(e0) == true);
            REQUIRE(manager.has<C>(e0) == false);
            REQUIRE(manager.has<A>(e1) == false);
            REQUIRE(manager.has<B>(e1) == true);
            REQUIRE(manager.has<C>(e1) == true);
            REQUIRE(manager.has<A>(e2) == false);
            REQUIRE(manager.has<B>(e2) == false);
            REQUIRE(manager.has<C>(e2) == true);
            REQUIRE(manager.get<B>(e0) == B{0});
            REQUIRE(manager.get<B>(e1) == B{1});
            REQUIRE(manager.get<C>(e1) == C{1});
            REQUIRE(manager.get<C>(e2) == C{2});
        }

        SECTION("clean") {
            manager.clean(e0);
            REQUIRE(manager.has<A>(e0) == false);
            REQUIRE(manager.has<B>(e0) == false);
            REQUIRE(manager.has<C>(e0) == false);
            REQUIRE(manager.has<A>(e1) == false);
            REQUIRE(manager.has<B>(e1) == true);
            REQUIRE(manager.has<C>(e1) == true);
            REQUIRE(manager.has<A>(e2) == true);
            REQUIRE(manager.has<B>(e2) == false);
            REQUIRE(manager.has<C>(e2) == true);

            manager.clean(e1);
            REQUIRE(manager.has<A>(e0) == false);
            REQUIRE(manager.has<B>(e0) == false);
            REQUIRE(manager.has<C>(e0) == false);
            REQUIRE(manager.has<A>(e1) == false);
            REQUIRE(manager.has<B>(e1) == false);
            REQUIRE(manager.has<C>(e1) == false);
            REQUIRE(manager.has<A>(e2) == true);
            REQUIRE(manager.has<B>(e2) == false);
            REQUIRE(manager.has<C>(e2) == true);

            manager.clean(e2);
            REQUIRE(manager.has<A>(e0) == false);
            REQUIRE(manager.has<B>(e0) == false);
            REQUIRE(manager.has<C>(e0) == false);
            REQUIRE(manager.has<A>(e1) == false);
            REQUIRE(manager.has<B>(e1) == false);
            REQUIRE(manager.has<C>(e1) == false);
            REQUIRE(manager.has<A>(e2) == false);
            REQUIRE(manager.has<B>(e2) == false);
            REQUIRE(manager.has<C>(e2) == false);
        }

        SECTION("view") {
            REQUIRE(isSameEntitySet(manager.view<A>(), EntityUSet{e0, e2}));
            REQUIRE(isSameEntitySet(manager.view<B>(), EntityUSet{e0, e1}));
            REQUIRE(isSameEntitySet(manager.view<C>(), EntityUSet{e1, e2}));
            REQUIRE(isSameEntitySet(manager.view<A, B>(), EntityUSet{e0}));
            REQUIRE(isSameEntitySet(manager.view<B, C>(), EntityUSet{e1}));
            REQUIRE(isSameEntitySet(manager.view<A, C>(), EntityUSet{e2}));
            REQUIRE(isSameEntitySet(manager.view<A, B, C>(), EntityUSet{}));
            REQUIRE(isSameEntitySet(manager.view<A>().exclude<C>(), EntityUSet{e0}));
        }
    }
}

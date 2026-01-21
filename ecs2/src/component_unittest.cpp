// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <catch2/catch_test_macros.hpp>
#include <internal/component.hpp>

#include <set>
#include <vector>

using namespace Istok::ECS;

namespace {

template <typename T>
std::set<size_t> indexSet(ComponentStorage<T>& storage) {
    return std::set(storage.indices().begin(), storage.indices().end());
}

}  // namespace

TEST_CASE("ComponentStorage - basics", "[unit][ecs]") {
    ComponentStorage<int> cs;
    REQUIRE(cs.size() == 0);
    REQUIRE(!cs.has(0));

    SECTION("insert first") {
        cs.insert(0, 42);
        REQUIRE(cs.has(0));
        REQUIRE(!cs.has(1));
        REQUIRE(cs.get(0) == 42);
        REQUIRE(indexSet(cs) == std::set<size_t>{0});
    }

    SECTION("insert middle") {
        cs.insert(2, 42);
        REQUIRE(!cs.has(0));
        REQUIRE(!cs.has(1));
        REQUIRE(cs.has(2));
        REQUIRE(!cs.has(3));
        REQUIRE(cs.get(2) == 42);
        REQUIRE(indexSet(cs) == std::set<size_t>{2});
    }

    SECTION("remove last") {
        cs.insert(0, 40);
        cs.insert(1, 41);
        cs.insert(2, 42);
        cs.insert(3, 43);
        cs.remove(3);
        REQUIRE(cs.has(0));
        REQUIRE(cs.has(1));
        REQUIRE(cs.has(2));
        REQUIRE(!cs.has(3));
        REQUIRE(cs.get(0) == 40);
        REQUIRE(cs.get(1) == 41);
        REQUIRE(cs.get(2) == 42);
        REQUIRE(indexSet(cs) == std::set<size_t>{0, 1, 2});
    }

    SECTION("remove middle") {
        cs.insert(0, 40);
        cs.insert(1, 41);
        cs.insert(2, 42);
        cs.insert(3, 43);
        cs.remove(1);
        REQUIRE(cs.has(0));
        REQUIRE(!cs.has(1));
        REQUIRE(cs.has(2));
        REQUIRE(cs.has(3));
        REQUIRE(cs.get(0) == 40);
        REQUIRE(cs.get(2) == 42);
        REQUIRE(cs.get(3) == 43);
        REQUIRE(indexSet(cs) == std::set<size_t>{0, 2, 3});
    }
}


TEST_CASE("ComponentStorage - multi actions", "[unit][ecs]") {
    ComponentStorage<int> cs;
    cs.insert(0, 100);
    cs.insert(3, 103);
    cs.insert(2, 102);
    cs.insert(9, 109);
    cs.insert(6, 106);
    cs.insert(7, 107);
    REQUIRE(indexSet(cs) == std::set<size_t>{0, 2, 3, 6, 7, 9});
    REQUIRE(cs.get(0) == 100);
    REQUIRE(cs.get(2) == 102);
    REQUIRE(cs.get(3) == 103);
    REQUIRE(cs.get(6) == 106);
    REQUIRE(cs.get(7) == 107);
    REQUIRE(cs.get(9) == 109);
    cs.remove(2);
    cs.remove(9);
    cs.remove(6);
    cs.insert(0, 200);
    cs.insert(5, 205);
    cs.insert(2, 202);
    cs.insert(8, 208);
    REQUIRE(indexSet(cs) == std::set<size_t>{0, 2, 3, 5, 7, 8});
    REQUIRE(cs.get(0) == 200);
    REQUIRE(cs.get(2) == 202);
    REQUIRE(cs.get(3) == 103);
    REQUIRE(cs.get(5) == 205);
    REQUIRE(cs.get(7) == 107);
    REQUIRE(cs.get(8) == 208);
}


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
}  // namespace


TEST_CASE("ComponentManager - basic", "[unit][ecs]") {
    ComponentManager cm;
    REQUIRE(!cm.has<A>(0));
    REQUIRE(!cm.has<B>(0));
    REQUIRE(!cm.has<C>(0));

    SECTION("insert single") {
        cm.insert(0, A{100});
        REQUIRE(cm.has<A>(0));
        REQUIRE(!cm.has<A>(1));
        REQUIRE(!cm.has<B>(0));
        REQUIRE(cm.get<A>(0) == A{100});
    }

    SECTION("insert multiple") {
        cm.insert(0, A{100});
        cm.insert(1, A{101});
        REQUIRE(cm.has<A>(0));
        REQUIRE(cm.has<A>(1));
        REQUIRE(!cm.has<A>(2));
        REQUIRE(!cm.has<B>(0));
        REQUIRE(cm.get<A>(0) == A{100});
        REQUIRE(cm.get<A>(1) == A{101});
    }

    SECTION("insert multiple components") {
        cm.insert(0, A{100});
        cm.insert(1, B{201});
        REQUIRE(cm.has<A>(0));
        REQUIRE(!cm.has<A>(1));
        REQUIRE(!cm.has<B>(0));
        REQUIRE(cm.has<B>(1));
        REQUIRE(cm.get<A>(0) == A{100});
        REQUIRE(cm.get<B>(1) == B{201});
    }

    SECTION("remove single") {
        cm.insert(0, A{100});
        cm.remove<A>(0);
        REQUIRE(!cm.has<A>(0));
    }

    SECTION("remove") {
        cm.insert(0, A{100});
        cm.insert(1, A{101});
        cm.insert(0, B{200});
        cm.insert(1, B{201});
        cm.remove<A>(0);
        REQUIRE(!cm.has<A>(0));
        REQUIRE(cm.has<A>(1));
        REQUIRE(cm.has<B>(0));
        REQUIRE(cm.has<B>(1));
        REQUIRE(cm.get<A>(1) == A{101});
        REQUIRE(cm.get<B>(0) == B{200});
        REQUIRE(cm.get<B>(1) == B{201});
    }
}

namespace {

template <typename T>
std::vector<size_t> vectorize(const T& x) {
    return std::vector<size_t>(x.begin(), x.end());
}

}  // namespace

TEST_CASE("ComponentManager - view", "[unit][ecs]") {
    ComponentManager cm;
    cm.insert(0, A{100});
    cm.insert(1, A{101});
    cm.insert(1, B{201});
    cm.insert(2, B{202});
    cm.insert(2, C{302});
    cm.insert(0, C{300});

    REQUIRE(vectorize(cm.view<A>()) == std::vector<size_t>{0, 1});
}

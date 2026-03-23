// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "istok/ecs.hpp"

#include <unordered_set>

#include <catch.hpp>
#include <catch2/trompeloeil.hpp>


using namespace Istok::ECS;
using trompeloeil::_;


TEST_CASE("ECSManager - entities", "[unit][ecs]") {
    ECSManager ecs;
    REQUIRE(ecs.countEntities() == 0);
    Entity a = ecs.createEntity();
    REQUIRE(ecs.countEntities() == 1);
    Entity b = ecs.createEntity();
    REQUIRE(ecs.countEntities() == 2);
    Entity c = ecs.createEntity();
    REQUIRE(ecs.countEntities() == 3);

    SECTION("entity validity") {
        REQUIRE(ecs.isValidEntity(a));
        REQUIRE(ecs.isValidEntity(b));
        REQUIRE(ecs.isValidEntity(c));
    }

    SECTION("entity uniqueness") {
        REQUIRE(a != b);
        REQUIRE(b != c);
        REQUIRE(c != a);
    }

    SECTION("invalidate on deletion") {
        ecs.deleteEntity(b);
        REQUIRE(ecs.countEntities() == 2);
        REQUIRE(ecs.isValidEntity(a));
        REQUIRE(!ecs.isValidEntity(b));
        REQUIRE(ecs.isValidEntity(c));
    }
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


TEST_CASE("ECSManager - components", "[unit][ecs]") {
    ECSManager ecs;
    Entity a = ecs.createEntity();
    Entity b = ecs.createEntity();
    Entity c = ecs.createEntity();

    SECTION("empty entity") {
        REQUIRE(!ecs.has<A>(a));
        REQUIRE(!ecs.has<B>(a));
        REQUIRE(!ecs.has<C>(a));
        REQUIRE(ecs.count<A>() == 0);
        REQUIRE(ecs.count<B>() == 0);
        REQUIRE(ecs.count<C>() == 0);
    }

    SECTION("single component") {
        ecs.insert(a, A{100});
        REQUIRE(ecs.count<A>() == 1);
        REQUIRE(ecs.has<A>(a));
        REQUIRE(ecs.get<A>(a) == A{100});
        REQUIRE(!ecs.has<A>(b));
    }

    SECTION("multiple components") {
        ecs.insert(a, A{100});
        ecs.insert(a, B{101});
        ecs.insert(b, A{200});
        ecs.insert(b, B{201});
        ecs.insert(c, C{300});
        REQUIRE(ecs.count<A>() == 2);
        REQUIRE(ecs.count<B>() == 2);
        REQUIRE(ecs.count<C>() == 1);
        REQUIRE(ecs.has<A>(a));
        REQUIRE(ecs.has<B>(a));
        REQUIRE(!ecs.has<C>(a));
        REQUIRE(ecs.has<A>(b));
        REQUIRE(ecs.has<B>(b));
        REQUIRE(!ecs.has<C>(b));
        REQUIRE(!ecs.has<A>(c));
        REQUIRE(!ecs.has<B>(c));
        REQUIRE(ecs.has<C>(c));
        REQUIRE(ecs.get<A>(a) == A{100});
        REQUIRE(ecs.get<B>(a) == B{101});
        REQUIRE(ecs.get<A>(b) == A{200});
        REQUIRE(ecs.get<B>(b) == B{201});
        REQUIRE(ecs.get<C>(c) == C{300});
    }

    SECTION("remove single") {
        ecs.insert(a, A{100});
        ecs.remove<A>(a);
        REQUIRE(ecs.count<A>() == 0);
        REQUIRE(!ecs.has<A>(a));
    }

    SECTION("remove") {
        ecs.insert(a, A{100});
        ecs.insert(b, A{101});
        ecs.insert(a, B{200});
        ecs.insert(b, B{201});
        REQUIRE(ecs.count<A>() == 2);
        REQUIRE(ecs.count<B>() == 2);
        ecs.remove<A>(a);
        REQUIRE(ecs.count<A>() == 1);
        REQUIRE(ecs.count<B>() == 2);
        REQUIRE(!ecs.has<A>(a));
        REQUIRE(ecs.has<A>(b));
        REQUIRE(ecs.has<B>(a));
        REQUIRE(ecs.has<B>(b));
        REQUIRE(ecs.get<A>(b) == A{101});
        REQUIRE(ecs.get<B>(a) == B{200});
        REQUIRE(ecs.get<B>(b) == B{201});
    }

    SECTION("remove all") {
        ecs.insert(a, A{100});
        ecs.insert(b, A{101});
        ecs.insert(a, B{200});
        ecs.insert(b, B{201});
        REQUIRE(ecs.count<A>() == 2);
        REQUIRE(ecs.count<B>() == 2);
        ecs.removeAll<A>();
        REQUIRE(ecs.count<A>() == 0);
        REQUIRE(ecs.count<B>() == 2);
        REQUIRE(!ecs.has<A>(a));
        REQUIRE(!ecs.has<A>(b));
        REQUIRE(ecs.has<B>(a));
        REQUIRE(ecs.has<B>(b));
        REQUIRE(ecs.get<B>(a) == B{200});
        REQUIRE(ecs.get<B>(b) == B{201});
    }

    SECTION("modify") {
        ecs.insert(a, A{100});
        ecs.get<A>(a) = A{101};
        REQUIRE(ecs.get<A>(a) == A{101});
        ecs.get<A>(a).value = 102;
        REQUIRE(ecs.get<A>(a) == A{102});
    }
}


namespace {
using EntitySet = std::unordered_set<Entity, Entity::Hasher>;

template <typename T>
EntitySet toEntitySet(const T& x) {
    return EntitySet(x.begin(), x.end());
}

}  // namespace


TEST_CASE("ECSManager - view", "[unit][ecs]") {
    ECSManager ecs;
    auto e0 = ecs.createEntity();
    auto e1 = ecs.createEntity();
    auto e2 = ecs.createEntity();
    auto e3 = ecs.createEntity();
    auto e4 = ecs.createEntity();
    auto e5 = ecs.createEntity();
    auto e6 = ecs.createEntity();
    ecs.insert(e0, A{0});
    ecs.insert(e1, A{0});
    ecs.insert(e2, A{0});
    ecs.insert(e3, A{0});
    ecs.insert(e2, B{0});
    ecs.insert(e3, B{0});
    ecs.insert(e4, B{0});
    ecs.insert(e5, B{0});
    ecs.insert(e0, C{0});
    ecs.insert(e2, C{0});
    ecs.insert(e4, C{0});
    ecs.insert(e6, C{0});

    REQUIRE(toEntitySet(ecs.view<A>()) == EntitySet{e0, e1, e2, e3});
    REQUIRE(toEntitySet(ecs.view<B>()) == EntitySet{e2, e3, e4, e5});
    REQUIRE(toEntitySet(ecs.view<C>()) == EntitySet{e0, e2, e4, e6});
    REQUIRE(toEntitySet(ecs.view<A, B>()) == EntitySet{e2, e3});
    REQUIRE(toEntitySet(ecs.view<B, A>()) == EntitySet{e2, e3});
    REQUIRE(toEntitySet(ecs.view<B, C>()) == EntitySet{e2, e4});
    REQUIRE(toEntitySet(ecs.view<C, B>()) == EntitySet{e2, e4});
    REQUIRE(toEntitySet(ecs.view<A, C>()) == EntitySet{e0, e2});
    REQUIRE(toEntitySet(ecs.view<C, A>()) == EntitySet{e0, e2});
    REQUIRE(toEntitySet(ecs.view<A, B, C>()) == EntitySet{e2});
    REQUIRE(toEntitySet(ecs.view<B, C, A>()) == EntitySet{e2});
    REQUIRE(toEntitySet(ecs.view<C, A, B>()) == EntitySet{e2});
}

TEST_CASE("ECSManager - empty view", "[unit][ecs]") {
    ECSManager ecs;
    auto e0 = ecs.createEntity();
    auto e1 = ecs.createEntity();
    auto e2 = ecs.createEntity();
    auto e3 = ecs.createEntity();

    REQUIRE(toEntitySet(ecs.view<A>()) == EntitySet{});

    ecs.insert(e0, A{10});
    ecs.insert(e1, A{11});
    ecs.insert(e2, B{22});
    ecs.insert(e3, B{23});

    REQUIRE(toEntitySet(ecs.view<A, B>()) == EntitySet{});
}


namespace {

struct Component {
    using Func = std::move_only_function<void() noexcept>;
    Func func_;
    Component(Func&& func) : func_(std::move(func)) {}
    ~Component() { func_(); }
};

struct MockComponent {
    using Type = std::unique_ptr<Component>;

    MAKE_MOCK0(kill, void(), noexcept);

    std::unique_ptr<Component> get() {
        return std::make_unique<Component>([this]() noexcept { kill(); });
    }
};

struct MockSystem {
    MAKE_MOCK1(run, void(ECSManager&), noexcept);
    MAKE_MOCK0(kill, void(), noexcept);

    System get() {
        auto item = std::make_unique<Component>(
            [this]() noexcept { kill(); });
        return [this, x=std::move(item)](ECSManager& ecs)
            noexcept { run(ecs); };
    }
};

}  // namespace


TEST_CASE("ECSManager - component lifecycle", "[unit][ecs]") {
    MockComponent ca, cb;
    auto ecs = std::make_unique<ECSManager>();
    auto a = ecs->createEntity();
    auto b = ecs->createEntity();
    ecs->insert(a, ca.get());
    ecs->insert(b, cb.get());

    SECTION("remove component") {
        {
            REQUIRE_CALL(ca, kill());
            ecs->remove<MockComponent::Type>(a);
        }
        {
            REQUIRE_CALL(cb, kill());
            ecs->remove<MockComponent::Type>(b);
        }
    }

    SECTION("delete entity") {
        {
            REQUIRE_CALL(ca, kill());
            ecs->deleteEntity(a);
        }
        {
            REQUIRE_CALL(cb, kill());
            ecs->deleteEntity(b);
        }
    }

    SECTION("remove all") {
        REQUIRE_CALL(ca, kill());
        REQUIRE_CALL(cb, kill());
        ecs->removeAll<MockComponent::Type>();
    }

    SECTION("destroy") {
        REQUIRE_CALL(ca, kill());
        REQUIRE_CALL(cb, kill());
        ecs.reset();
    }
}


TEST_CASE("ECSManager - system lifecycle", "[unit][ecs]") {
    MockSystem loop1;
    MockSystem loop2;
    MockSystem head1;
    MockSystem head2;
    MockSystem tail1;
    MockSystem tail2;
    MockComponent comp;

    auto ecs = std::make_unique<ECSManager>();
    auto* ecsPtr = ecs.get();
    Entity a = ecs->createEntity();
    Entity b = ecs->createEntity();
    ecs->insert(a, A{100});
    ecs->insert(b, comp.get());
    ecs->addLoopSystem(loop1.get());
    ecs->addLoopSystem(loop2.get());
    ecs->addHeadCleanupSystem(head1.get());
    ecs->addHeadCleanupSystem(head2.get());
    ecs->addTailCleanupSystem(tail1.get());
    ecs->addTailCleanupSystem(tail2.get());

    {
        trompeloeil::sequence seq;
        REQUIRE_CALL(loop2, kill()).IN_SEQUENCE(seq);
        REQUIRE_CALL(loop1, kill()).IN_SEQUENCE(seq);
        REQUIRE_CALL(head1, run(_)).WITH(&_1 == ecsPtr).IN_SEQUENCE(seq);
        REQUIRE_CALL(head1, kill()).IN_SEQUENCE(seq);
        REQUIRE_CALL(head2, run(_)).WITH(&_1 == ecsPtr).IN_SEQUENCE(seq);
        REQUIRE_CALL(head2, kill()).IN_SEQUENCE(seq);
        REQUIRE_CALL(tail2, run(_)).WITH(&_1 == ecsPtr).IN_SEQUENCE(seq);
        REQUIRE_CALL(tail2, kill()).IN_SEQUENCE(seq);
        REQUIRE_CALL(tail1, run(_)).WITH(&_1 == ecsPtr).IN_SEQUENCE(seq);
        REQUIRE_CALL(tail1, kill()).IN_SEQUENCE(seq);
        REQUIRE_CALL(comp, kill()).IN_SEQUENCE(seq);
        ecs.reset();
    }
}


TEST_CASE("ECSManager - loop", "[unit][ecs]") {
    MockSystem loop1;
    MockSystem loop2;
    MockSystem loop3;
    ALLOW_CALL(loop1, kill());
    ALLOW_CALL(loop2, kill());
    ALLOW_CALL(loop3, kill());

    auto ecs = std::make_unique<ECSManager>();
    auto* ecsPtr = ecs.get();
    ecs->addLoopSystem(loop1.get());
    ecs->addLoopSystem(loop2.get());
    ecs->addLoopSystem(loop3.get());

    for (size_t i = 0; i < 3; ++ i) {
        trompeloeil::sequence seq;
        REQUIRE_CALL(loop1, run(_)).WITH(&_1 == ecsPtr).IN_SEQUENCE(seq);
        REQUIRE_CALL(loop2, run(_)).WITH(&_1 == ecsPtr).IN_SEQUENCE(seq);
        REQUIRE_CALL(loop3, run(_)).WITH(&_1 == ecsPtr).IN_SEQUENCE(seq);
        ecs->iterate();
    }

    {
        trompeloeil::sequence seq;
        REQUIRE_CALL(loop1, run(_)).WITH(&_1 == ecsPtr).IN_SEQUENCE(seq)
            .LR_SIDE_EFFECT([&]() {
                trompeloeil::sequence seq1;
                REQUIRE_CALL(loop2, run(_)).WITH(&_1 == ecsPtr)
                    .IN_SEQUENCE(seq1);
                REQUIRE_CALL(loop3, run(_)).WITH(&_1 == ecsPtr)
                    .IN_SEQUENCE(seq1);
                ecs->iterate();
            });
        REQUIRE_CALL(loop2, run(_)).WITH(&_1 == ecsPtr).IN_SEQUENCE(seq);
        REQUIRE_CALL(loop3, run(_)).WITH(&_1 == ecsPtr).IN_SEQUENCE(seq);
        ecs->iterate();
    }

    {
        trompeloeil::sequence seq;
        REQUIRE_CALL(loop1, run(_)).WITH(&_1 == ecsPtr).IN_SEQUENCE(seq);
        REQUIRE_CALL(loop2, run(_)).WITH(&_1 == ecsPtr).IN_SEQUENCE(seq)
            .LR_SIDE_EFFECT([&]() {
                trompeloeil::sequence seq1;
                REQUIRE_CALL(loop3, run(_)).WITH(&_1 == ecsPtr)
                    .IN_SEQUENCE(seq1);
                REQUIRE_CALL(loop1, run(_)).WITH(&_1 == ecsPtr)
                    .IN_SEQUENCE(seq1);
                ecs->iterate();
            });
        REQUIRE_CALL(loop3, run(_)).WITH(&_1 == ecsPtr).IN_SEQUENCE(seq);
        ecs->iterate();
    }

    {
        trompeloeil::sequence seq;
        REQUIRE_CALL(loop1, run(_)).WITH(&_1 == ecsPtr).IN_SEQUENCE(seq);
        REQUIRE_CALL(loop2, run(_)).WITH(&_1 == ecsPtr).IN_SEQUENCE(seq);
        REQUIRE_CALL(loop3, run(_)).WITH(&_1 == ecsPtr).IN_SEQUENCE(seq)
            .LR_SIDE_EFFECT([&]() {
                trompeloeil::sequence seq1;
                REQUIRE_CALL(loop1, run(_)).WITH(&_1 == ecsPtr)
                    .IN_SEQUENCE(seq1);
                REQUIRE_CALL(loop2, run(_)).WITH(&_1 == ecsPtr)
                    .IN_SEQUENCE(seq1);
                ecs->iterate();
            });
        ecs->iterate();
    }
}

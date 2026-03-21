// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "istok/ecs.hpp"


#include <string>
#include <unordered_set>

using namespace Istok::ECS;


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

template <typename Tag>
class MockUnique {
public:
    MockUnique(std::string& status) : status_(&status) {
        *status_ = "valid";
    }

    ~MockUnique() {
        clear();
    }

    MockUnique(const MockUnique&) = delete;
    MockUnique& operator=(const MockUnique&) = delete;

    MockUnique(MockUnique&& other) : status_(other.status_) {
        other.status_ = nullptr;
    }

    MockUnique& operator=(MockUnique&& other) {
        if (this != &other) {
            clear();
            this->status_ = other.status_;
            other.status_ = nullptr;
        }
        return *this;
    }

private:
    std::string* status_ = nullptr;

    void clear() {
        if (status_) {
            *status_ = "destroyed";
        }
    }
};

}  // namespace


TEST_CASE("ECSManager - component lifecycle", "[unit][ecs]") {
    ECSManager ecs;
    using CA = MockUnique<A>;
    auto a = ecs.createEntity();
    auto b = ecs.createEntity();
    std::string sa;
    std::string sb;
    ecs.insert(a, CA(sa));
    ecs.insert(b, CA(sb));
    REQUIRE(sa == "valid");
    REQUIRE(sb == "valid");

    SECTION("remove component") {
        ecs.remove<CA>(a);
        REQUIRE(sa == "destroyed");
        REQUIRE(sb == "valid");
    }

    SECTION("remove all") {
        ecs.removeAll<CA>();
        REQUIRE(sa == "destroyed");
        REQUIRE(sb == "destroyed");
    }

    SECTION("delete entity") {
        ecs.deleteEntity(a);
        REQUIRE(sa == "destroyed");
        REQUIRE(sb == "valid");
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


TEST_CASE("ECSManager - loop systems", "[unit][ecs]") {
    ECSManager ecs;

    SECTION("single") {
        auto a = ecs.createEntity();
        ecs.insert(a, std::string("a"));
        ecs.addLoopSystem([a](ECSManager& ecs) noexcept {
            ecs.get<std::string>(a) += "1"; });
        REQUIRE(ecs.get<std::string>(a) == "a");
        ecs.iterate();
        REQUIRE(ecs.get<std::string>(a) == "a1");
        ecs.iterate();
        REQUIRE(ecs.get<std::string>(a) == "a11");
    }

    SECTION("multiple") {
        auto a = ecs.createEntity();
        ecs.insert(a, std::string("a"));
        ecs.addLoopSystem([a](ECSManager& ecs) noexcept {
            ecs.get<std::string>(a) += "1"; });
        ecs.addLoopSystem([a](ECSManager& ecs) noexcept {
            ecs.get<std::string>(a) += "2"; });
        REQUIRE(ecs.get<std::string>(a) == "a");
        ecs.iterate();
        REQUIRE(ecs.get<std::string>(a) == "a12");
        ecs.iterate();
        REQUIRE(ecs.get<std::string>(a) == "a1212");
    }
}


TEST_CASE("ECSManager - cleanup systems", "[unit][ecs]") {
    SECTION("single") {
        std::string log = "x";
        {
            ECSManager ecs;
            auto a = ecs.createEntity();
            ecs.insert(a, std::string("a"));
            ecs.addTailCleanupSystem([a, &log](ECSManager& ecs) noexcept {
                log += ecs.get<std::string>(a); });
            REQUIRE(log == "x");
        }
        REQUIRE(log == "xa");
    }

    SECTION("multiple") {
        std::string log = "x";
        {
            ECSManager ecs;
            auto a = ecs.createEntity();
            ecs.insert(a, std::string("a"));
            ecs.addTailCleanupSystem([a, &log](ECSManager& ecs) noexcept {
                log += ecs.get<std::string>(a) + "1"; });
            ecs.addTailCleanupSystem([a, &log](ECSManager& ecs) noexcept {
                log += ecs.get<std::string>(a) + "2"; });
            REQUIRE(log == "x");
        }
        REQUIRE(log == "xa2a1");
    }
}


TEST_CASE("ECSManager - all systems", "[unit][ecs]") {
    std::string log = "x";
    {
        ECSManager ecs;
        auto a = ecs.createEntity();
        ecs.insert(a, std::string("a"));
        ecs.addLoopSystem([a, &log](ECSManager& ecs) noexcept {
            log += ecs.get<std::string>(a) + "1"; });
        ecs.addLoopSystem([a, &log](ECSManager& ecs) noexcept {
            log += ecs.get<std::string>(a) + "2"; });
        ecs.addTailCleanupSystem([a, &log](ECSManager& ecs) noexcept {
            log += ecs.get<std::string>(a) + "5"; });
        ecs.addTailCleanupSystem([a, &log](ECSManager& ecs) noexcept {
            log += ecs.get<std::string>(a) + "6"; });
        REQUIRE(log == "x");
        ecs.iterate();
        REQUIRE(log == "xa1a2");
        ecs.iterate();
        REQUIRE(log == "xa1a2a1a2");
    }
    REQUIRE(log == "xa1a2a1a2a6a5");
}


using trompeloeil::_;

namespace {

struct MockSystems {
    MAKE_MOCK1(loop1, void(ECSManager&), noexcept);
    MAKE_MOCK1(cleanH1, void(ECSManager&), noexcept);
    MAKE_MOCK1(cleanT1, void(ECSManager&), noexcept);
    MAKE_MOCK1(loop2, void(ECSManager&), noexcept);
    MAKE_MOCK1(cleanH2, void(ECSManager&), noexcept);
    MAKE_MOCK1(cleanT2, void(ECSManager&), noexcept);

    void addTo(ECSManager& ecs) {
        ecs.addLoopSystem(
            [this](ECSManager& ecs) noexcept { loop1(ecs); });
        ecs.addHeadCleanupSystem(
            [this](ECSManager& ecs) noexcept { cleanH1(ecs); });
        ecs.addTailCleanupSystem(
            [this](ECSManager& ecs) noexcept { cleanT1(ecs); });
        ecs.addLoopSystem(
            [this](ECSManager& ecs) noexcept { loop2(ecs); });
        ecs.addHeadCleanupSystem(
            [this](ECSManager& ecs) noexcept { cleanH2(ecs); });
        ecs.addTailCleanupSystem(
            [this](ECSManager& ecs) noexcept { cleanT2(ecs); });
    }
};

}  // namespace

TEST_CASE("ECSManager - clear", "[unit][ecs]") {
    ECSManager ecs;
    Entity a = ecs.createEntity();
    Entity b = ecs.createEntity();
    Entity c = ecs.createEntity();
    ecs.insert(a, A{100});
    ecs.insert(a, B{101});
    ecs.insert(b, A{200});
    ecs.insert(b, B{201});
    ecs.insert(c, C{300});
    MockSystems mock;
    mock.addTo(ecs);
    trompeloeil::sequence seq;
    {
        REQUIRE_CALL(mock, cleanH1(_)).LR_WITH(&_1 == &ecs).IN_SEQUENCE(seq);
        REQUIRE_CALL(mock, cleanH2(_)).LR_WITH(&_1 == &ecs).IN_SEQUENCE(seq);
        REQUIRE_CALL(mock, cleanT2(_)).LR_WITH(&_1 == &ecs).IN_SEQUENCE(seq);
        REQUIRE_CALL(mock, cleanT1(_)).LR_WITH(&_1 == &ecs).IN_SEQUENCE(seq);
        ecs.clear();
    }
    REQUIRE(ecs.countEntities() == 0);
    REQUIRE(ecs.count<A>() == 0);
    REQUIRE(ecs.count<B>() == 0);
    REQUIRE(ecs.count<C>() == 0);
    REQUIRE(!ecs.isValidEntity(a));
    REQUIRE(!ecs.isValidEntity(b));
    REQUIRE(!ecs.isValidEntity(c));

    // Ensure mock systems are not called
    ecs.iterate();
    ecs.clear();
}

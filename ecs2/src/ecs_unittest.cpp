// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <catch2/catch_test_macros.hpp>
#include <ecs.hpp>

using namespace Istok::ECS;


TEST_CASE("ECSManager - entities", "[unit][ecs]") {
    ECSManager ecs;
    Entity a = ecs.createEntity();
    Entity b = ecs.createEntity();
    Entity c = ecs.createEntity();

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
    }

    SECTION("single component") {
        ecs.insert(a, A{100});
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
        REQUIRE(!ecs.has<A>(a));
    }

    SECTION("remove") {
        ecs.insert(a, A{100});
        ecs.insert(b, A{101});
        ecs.insert(a, B{200});
        ecs.insert(b, B{201});
        ecs.remove<A>(a);
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
        ecs.removeAll<A>();
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

template <typename T>
std::set<size_t> toSet(const T& x) {
    return std::set<size_t>(x.begin(), x.end());
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

    REQUIRE(toSet(ecs.view<A>()) == std::set<size_t>{0, 1, 2, 3});
    REQUIRE(toSet(ecs.view<B>()) == std::set<size_t>{2, 3, 4, 5});
    REQUIRE(toSet(ecs.view<C>()) == std::set<size_t>{0, 2, 4, 6});
    REQUIRE(toSet(ecs.view<A, B>()) == std::set<size_t>{2, 3});
    REQUIRE(toSet(ecs.view<B, A>()) == std::set<size_t>{2, 3});
    REQUIRE(toSet(ecs.view<B, C>()) == std::set<size_t>{2, 4});
    REQUIRE(toSet(ecs.view<C, B>()) == std::set<size_t>{2, 4});
    REQUIRE(toSet(ecs.view<A, C>()) == std::set<size_t>{0, 2});
    REQUIRE(toSet(ecs.view<C, A>()) == std::set<size_t>{0, 2});
    REQUIRE(toSet(ecs.view<A, B, C>()) == std::set<size_t>{2});
    REQUIRE(toSet(ecs.view<B, C, A>()) == std::set<size_t>{2});
    REQUIRE(toSet(ecs.view<C, A, B>()) == std::set<size_t>{2});
}

TEST_CASE("ECSManager - empty view", "[unit][ecs]") {
    ECSManager ecs;
    auto e0 = ecs.createEntity();
    auto e1 = ecs.createEntity();
    auto e2 = ecs.createEntity();
    auto e3 = ecs.createEntity();

    REQUIRE(toSet(ecs.view<A>()) == std::set<size_t>{});

    ecs.insert(e0, A{10});
    ecs.insert(e1, A{11});
    ecs.insert(e2, B{22});
    ecs.insert(e3, B{23});

    REQUIRE(toSet(ecs.view<A, B>()) == std::set<size_t>{});
}

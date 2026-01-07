// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <internal/entity.hpp>

#include <set>

using namespace Istok::ECS;

TEST_CASE("EntityManager - basics", "[unit][ecs]") {
    EntityManager em;
    Entity a = em.createEntity();
    Entity b = em.createEntity();
    Entity c = em.createEntity();

    SECTION("indices") {
        REQUIRE(a.index() == 0);
        REQUIRE(b.index() == 1);
        REQUIRE(c.index() == 2);
    }
    
    SECTION("validity") {
        REQUIRE(em.isValidEntity(a));
        REQUIRE(em.isValidEntity(b));
        REQUIRE(em.isValidEntity(c));
    }

    SECTION("uniqueness") {
        REQUIRE(a != b);
        REQUIRE(b != c);
        REQUIRE(c != a);
    }

    SECTION("invalidate on deletion") {
        em.deleteEntity(b);
        REQUIRE(em.isValidEntity(a));
        REQUIRE(!em.isValidEntity(b));
        REQUIRE(em.isValidEntity(c));
    }

    SECTION("deletion is idempotent") {
        em.deleteEntity(b);
        em.deleteEntity(b);
        REQUIRE(em.isValidEntity(a));
        REQUIRE(!em.isValidEntity(b));
        REQUIRE(em.isValidEntity(c));
    }

    SECTION("reuse after deletion") {
        em.deleteEntity(b);
        Entity d = em.createEntity();
        REQUIRE(em.isValidEntity(a));
        REQUIRE(!em.isValidEntity(b));
        REQUIRE(em.isValidEntity(c));
        REQUIRE(em.isValidEntity(d));
        REQUIRE(d != a);
        REQUIRE(d != b);
        REQUIRE(d != c);
        REQUIRE(d.index() == b.index());
    }
}


TEST_CASE("EntityManager - mass index reuse", "[unit][ecs]") {
    EntityManager em;
    Entity a = em.createEntity();
    Entity b = em.createEntity();
    Entity c = em.createEntity();
    em.deleteEntity(a);
    em.deleteEntity(b);
    em.deleteEntity(c);
    Entity d = em.createEntity();
    Entity e = em.createEntity();
    Entity f = em.createEntity();
    em.deleteEntity(d);
    em.deleteEntity(e);
    em.deleteEntity(f);
    Entity g = em.createEntity();
    Entity h = em.createEntity();
    Entity i = em.createEntity();
    REQUIRE(!em.isValidEntity(a));
    REQUIRE(!em.isValidEntity(b));
    REQUIRE(!em.isValidEntity(c));
    REQUIRE(!em.isValidEntity(d));
    REQUIRE(!em.isValidEntity(e));
    REQUIRE(!em.isValidEntity(f));
    REQUIRE(em.isValidEntity(g));
    REQUIRE(em.isValidEntity(h));
    REQUIRE(em.isValidEntity(i));
    std::set<uint32_t> s = {0, 1, 2};
    std::set<uint32_t> x = {a.index(), b.index(), c.index()};
    std::set<uint32_t> y = {d.index(), e.index(), f.index()};
    std::set<uint32_t> z = {g.index(), h.index(), i.index()};
    REQUIRE(x == s);
    REQUIRE(y == s);
    REQUIRE(z == s);
}

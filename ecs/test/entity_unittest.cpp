// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "istok/ecs/entity.hpp"

#include <catch.hpp>

using namespace Istok::ECS;
using namespace Istok::ECS::Internal;

TEST_CASE("EntityManager - basics", "[unit][ecs]") {
    EntityManager em;
    Entity a = em.create();
    Entity b = em.create();
    Entity c = em.create();

    REQUIRE(a.index() == 0);
    REQUIRE(b.index() == 1);
    REQUIRE(c.index() == 2);

    REQUIRE(em.isValid(a));
    REQUIRE(em.isValid(b));
    REQUIRE(em.isValid(c));

    REQUIRE(a != b);
    REQUIRE(b != c);
    REQUIRE(c != a);

    REQUIRE(em.get(0) == a);
    REQUIRE(em.get(1) == b);
    REQUIRE(em.get(2) == c);

    em.remove(b);
    REQUIRE(em.isValid(a));
    REQUIRE(!em.isValid(b));
    REQUIRE(em.isValid(c));

    Entity d = em.create();
    REQUIRE(d.index() == b.index());
    REQUIRE(!em.isValid(b));
    REQUIRE(em.isValid(d));
    REQUIRE(d != a);
    REQUIRE(d != b);
    REQUIRE(d != c);
}


TEST_CASE("EntityManager - mass index reuse", "[unit][ecs]") {
    EntityManager em;
    Entity a = em.create();
    Entity b = em.create();
    Entity c = em.create();
    em.remove(a);
    em.remove(b);
    em.remove(c);
    Entity d = em.create();
    Entity e = em.create();
    Entity f = em.create();
    em.remove(d);
    em.remove(e);
    em.remove(f);
    Entity g = em.create();
    Entity h = em.create();
    Entity i = em.create();
    REQUIRE(!em.isValid(a));
    REQUIRE(!em.isValid(b));
    REQUIRE(!em.isValid(c));
    REQUIRE(!em.isValid(d));
    REQUIRE(!em.isValid(e));
    REQUIRE(!em.isValid(f));
    REQUIRE(em.isValid(g));
    REQUIRE(em.isValid(h));
    REQUIRE(em.isValid(i));
    REQUIRE(em.get(0) == g);
    REQUIRE(em.get(1) == h);
    REQUIRE(em.get(2) == i);
    std::set<size_t> s = {0, 1, 2};
    std::set<size_t> x = {a.index(), b.index(), c.index()};
    std::set<size_t> y = {d.index(), e.index(), f.index()};
    std::set<size_t> z = {g.index(), h.index(), i.index()};
    REQUIRE(x == s);
    REQUIRE(y == s);
    REQUIRE(z == s);
}

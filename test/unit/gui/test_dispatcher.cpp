// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/core/dispatcher.hpp>


#include <string>


namespace {

    class A {
    public:
        std::string value;
        virtual ~A() {}
    };

    class AA: public A {};
    class AB: public A {};
    class AAA: public AA {};
    class AAB: public AA {};
    class ABA: public AB {};
    class ABB: public AB {};

    class FakeDispatcherSingle: public Dispatcher<A> {
    public:
        FakeDispatcherSingle() {
            init(&FakeDispatcherSingle::handleA);
        }

        void handleA(A& target) {
            target.value = "A";
        }
    };

    class FakeDispatcherFlat: public Dispatcher<A> {
    public:
        FakeDispatcherFlat() {
            init(
                &FakeDispatcherFlat::handleAA,
                &FakeDispatcherFlat::handleAB
            );
        }

        void handleAA(AA& target) {
            target.value = "AA";
        }

        void handleAB(AB& target) {
            target.value = "AB";
        }
    };

    class FakeDispatcherFallback: public Dispatcher<A> {
    public:
        FakeDispatcherFallback() {
            init(
                &FakeDispatcherFallback::handleA,
                &FakeDispatcherFallback::handleAA
            );
        }

        void handleA(A& target) {
            target.value = "A";
        }

        void handleAA(AA& target) {
            target.value = "AA";
        }
    };

    class FakeDispatcherComplex: public Dispatcher<A> {
    public:
        FakeDispatcherComplex() {
            init(
                &FakeDispatcherComplex::handleAA,
                &FakeDispatcherComplex::handleA,
                &FakeDispatcherComplex::handleAAA,
                &FakeDispatcherComplex::handleABB
            );
        }

        void handleA(A& target) {
            target.value = "A";
        }

        void handleAA(AA& target) {
            target.value = "AA";
        }

        void handleAAA(AAA& target) {
            target.value = "AAA";
        }

        void handleABB(ABB& target) {
            target.value = "ABB";
        }
    };
}


TEST_CASE("Dispatcher - single", "[unit][gui]") {
    FakeDispatcherSingle dispatcher;
    A a1, a2;
    REQUIRE(a1.value == "");
    REQUIRE(a2.value == "");
    dispatcher(a1);
    dispatcher(a2);
    REQUIRE(a1.value == "A");
    REQUIRE(a2.value == "A");
}


TEST_CASE("Dispatcher - flat", "[unit][gui]") {
    FakeDispatcherFlat dispatcher;
    AA aa;
    AB ab;
    REQUIRE(aa.value == "");
    REQUIRE(ab.value == "");
    dispatcher(aa);
    dispatcher(ab);
    REQUIRE(aa.value == "AA");
    REQUIRE(ab.value == "AB");
}


TEST_CASE("Dispatcher - fallback", "[unit][gui]") {
    FakeDispatcherFallback dispatcher;
    A a;
    AA aa;
    AB ab;
    REQUIRE(a.value == "");
    REQUIRE(aa.value == "");
    REQUIRE(ab.value == "");
    dispatcher(a);
    dispatcher(aa);
    dispatcher(ab);
    REQUIRE(a.value == "A");
    REQUIRE(aa.value == "AA");
    REQUIRE(ab.value == "A");
}


TEST_CASE("Dispatcher - complex", "[unit][gui]") {
    FakeDispatcherComplex dispatcher;
    A a;
    AA aa;
    AB ab;
    AAA aaa;
    AAB aab;
    ABA aba;
    ABB abb;
    REQUIRE(a.value == "");
    REQUIRE(aa.value == "");
    REQUIRE(ab.value == "");
    REQUIRE(aaa.value == "");
    REQUIRE(aab.value == "");
    REQUIRE(aba.value == "");
    REQUIRE(abb.value == "");
    dispatcher(a);
    dispatcher(aa);
    dispatcher(ab);
    dispatcher(aaa);
    dispatcher(aab);
    dispatcher(aba);
    dispatcher(abb);
    REQUIRE(a.value == "A");
    REQUIRE(aa.value == "AA");
    REQUIRE(ab.value == "A");
    REQUIRE(aaa.value == "AAA");
    REQUIRE(aab.value == "AA");
    REQUIRE(aba.value == "A");
    REQUIRE(abb.value == "ABB");
}

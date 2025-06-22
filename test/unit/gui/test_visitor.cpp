// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui/core/visitor.hpp>


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

    class MockVisitorSingle: public Visitor<A> {
    public:
        MockVisitorSingle() {
            init(&MockVisitorSingle::visitA);
        }

        void visitA(A& target) {
            target.value = "A";
        }
    };

    class MockVisitorFlat: public Visitor<A> {
    public:
        MockVisitorFlat() {
            init(
                &MockVisitorFlat::visitAA,
                &MockVisitorFlat::visitAB
            );
        }

        void visitAA(AA& target) {
            target.value = "AA";
        }

        void visitAB(AB& target) {
            target.value = "AB";
        }
    };

    class MockVisitorFallback: public Visitor<A> {
    public:
        MockVisitorFallback() {
            init(
                &MockVisitorFallback::visitA,
                &MockVisitorFallback::visitAA
            );
        }

        void visitA(A& target) {
            target.value = "A";
        }

        void visitAA(AA& target) {
            target.value = "AA";
        }
    };

    class MockVisitorComplex: public Visitor<A> {
    public:
        MockVisitorComplex() {
            init(
                &MockVisitorComplex::visitAA,
                &MockVisitorComplex::visitA,
                &MockVisitorComplex::visitAAA,
                &MockVisitorComplex::visitABB
            );
        }

        void visitA(A& target) {
            target.value = "A";
        }

        void visitAA(AA& target) {
            target.value = "AA";
        }

        void visitAAA(AAA& target) {
            target.value = "AAA";
        }

        void visitABB(ABB& target) {
            target.value = "ABB";
        }
    };
}


TEST_CASE("Visitor - single", "[unit][gui]") {
    MockVisitorSingle visitor;
    A a1, a2;
    REQUIRE(a1.value == "");
    REQUIRE(a2.value == "");
    visitor.visit(a1);
    visitor.visit(a2);
    REQUIRE(a1.value == "A");
    REQUIRE(a2.value == "A");
}


TEST_CASE("Visitor - flat", "[unit][gui]") {
    MockVisitorFlat visitor;
    AA aa;
    AB ab;
    REQUIRE(aa.value == "");
    REQUIRE(ab.value == "");
    visitor.visit(aa);
    visitor.visit(ab);
    REQUIRE(aa.value == "AA");
    REQUIRE(ab.value == "AB");
}


TEST_CASE("Visitor - fallback", "[unit][gui]") {
    MockVisitorFallback visitor;
    A a;
    AA aa;
    AB ab;
    REQUIRE(a.value == "");
    REQUIRE(aa.value == "");
    REQUIRE(ab.value == "");
    visitor.visit(a);
    visitor.visit(aa);
    visitor.visit(ab);
    REQUIRE(a.value == "A");
    REQUIRE(aa.value == "AA");
    REQUIRE(ab.value == "A");
}


TEST_CASE("Visitor - complex", "[unit][gui]") {
    MockVisitorComplex visitor;
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
    visitor.visit(a);
    visitor.visit(aa);
    visitor.visit(ab);
    visitor.visit(aaa);
    visitor.visit(aab);
    visitor.visit(aba);
    visitor.visit(abb);
    REQUIRE(a.value == "A");
    REQUIRE(aa.value == "AA");
    REQUIRE(ab.value == "A");
    REQUIRE(aaa.value == "AAA");
    REQUIRE(aab.value == "AA");
    REQUIRE(aba.value == "A");
    REQUIRE(abb.value == "ABB");
}

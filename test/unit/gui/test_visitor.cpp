// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui\visitor.hpp>


#include <string>


namespace {

    class A {
    public:
        std::string value;
        virtual ~A() {}
    };

    class AA: public A {};
    class AB: public A {};

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

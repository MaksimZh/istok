// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui\visitor.hpp>


#include <string>


namespace {

    class A {
    public:
        std::string value;
    };

    class MockVisitor1: public Visitor<A> {
    public:
        MockVisitor1() {
            registerMethods(&MockVisitor1::visitA);
        }

        void visitA(A& target) {
            target.value = "A";
        }
    };
}


TEST_CASE("Visitor - single", "[unit][gui]") {
    MockVisitor1 visitor;
    A a1, a2;
    REQUIRE(a1.value == "");
    REQUIRE(a2.value == "");
    visitor.visit(a1);
    visitor.visit(a2);
    REQUIRE(a1.value == "A");
    REQUIRE(a2.value == "A");
}

// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui\visitor.hpp>


namespace {

    class A {};

}


TEST_CASE("Visitor", "[unit][gui]") {
    Visitor<A> visitor;
}

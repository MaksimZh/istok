// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <tools/helpers.hpp>

using namespace Istok::Tools;

#include <type_traits>
#include <thread>

namespace {
struct FakeHashed {
    struct Hasher {
        size_t operator()(const FakeHashed& item) const {
            return 0;
        }
    };
};
}

static_assert(std::same_as<hash<int>, std::hash<int>>);
static_assert(std::same_as<hash<FakeHashed>, FakeHashed::Hasher>);


TEST_CASE("Tools - Instance getter", "[unit][tools]") {
    class Foo {
    public:
        Foo() : instanceGetter(this) {}

        static Foo* release() {
            return InstanceGetter<Foo>::release();
        }
    
    private:
        InstanceGetter<Foo> instanceGetter;
    };

    REQUIRE(Foo::release() == nullptr);

    SECTION("simple constructor") {
        Foo a;
        REQUIRE(Foo::release() == &a);
    }
    
    SECTION("unlock on destroy") {
        { Foo b; }
        REQUIRE(Foo::release() == nullptr);
        Foo c;
        REQUIRE(Foo::release() == &c);
    }

    SECTION("threads") {
        Foo* p1 = nullptr;
        Foo* p2 = nullptr;
        Foo* r1 = nullptr;
        Foo* r2 = nullptr;
        std::thread t1([&]{
            Foo a;
            p1 = &a;
            r1 = Foo::release();
        });
        std::thread t2([&]{
            Foo a;
            p2 = &a;
            r2 = Foo::release();
        });
        t1.join();
        t2.join();
        REQUIRE(r1 == p1);
        REQUIRE(r2 == p2);
    }
}

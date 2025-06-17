// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui\dispatcher.hpp>

#include <memory>

namespace {
    struct T {
        std::string id;
        std::string value;

        T(std::string id) : id(id) {}
    };

    using FakeKeyFunc = std::function<std::string(const T&)>;
    
    class FakeDispatcher:
        public Dispatcher<T, FakeKeyFunc> {
    public:
        template<typename D1>
        class FakeCaller: public SubCaller<D1> {
        public:
            FakeCaller(MethodPtr<D1, T> method)
                : SubCaller<D1>(method) {}
            
            bool fits(T& target) override {
                return true;
            }

            std::string key() override {
                return "";
            }
        };

        FakeDispatcher() : Dispatcher([](const T& v) { return v.id; }) {}

        template <typename... Args>
        void init(Args... args) {
            Dispatcher::init(FakeCaller(std::forward<Args>(args))...);
        }
    };
    
    class FakeDispatcherSimple: public FakeDispatcher {
    public:
        FakeDispatcherSimple() {
            init(
                &FakeDispatcherSimple::processA,
                &FakeDispatcherSimple::processB);
        }

        void processA(T& arg) {
            arg.value = "a";
        }

        void processB(T& arg) {
            arg.value = "b";
        }
    };
}


TEST_CASE("Dispatcher - single", "[unit][gui]") {
    FakeDispatcherSimple dispatcher;
    T a("A");
    T b("B");
    REQUIRE(a.value == "");
    REQUIRE(b.value == "");
    dispatcher(a);
    dispatcher(b);
    REQUIRE(a.value == "a");
    REQUIRE(b.value == "b");
}

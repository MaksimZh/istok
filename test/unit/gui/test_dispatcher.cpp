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
        FakeDispatcher(std::unique_ptr<Caller<Dispatcher<T, FakeKeyFunc>, T>> method)
            : Dispatcher(
                [](const T& v) { return v.id; },
                std::move(method)
            ) {}
    };

    template<typename D1>
    class FakeCaller: public SubCaller<Dispatcher<T, FakeKeyFunc>, T, D1> {
    public:
        FakeCaller(MethodPtr<D1, T> method) 
            : SubCaller<Dispatcher<T, FakeKeyFunc>, T, D1>(method) {}
        
        bool fits(T& target) override {
            return true;
        }
    };

    template<typename D1>
    std::unique_ptr<Caller<Dispatcher<T, FakeKeyFunc>, T>> caller(MethodPtr<D1, T> method) {
        return std::unique_ptr<Caller<Dispatcher<T, FakeKeyFunc>, T>>(
            new FakeCaller<D1>(method));
    }

    
    class FakeDispatcherSingle: public FakeDispatcher {
    public:
        FakeDispatcherSingle()
            : FakeDispatcher(caller(&FakeDispatcherSingle::processA)) {}
        
        void processA(T& arg) {
            arg.value = "a";
        }
    };
}


TEST_CASE("Dispatcher - single", "[unit][gui]") {
    FakeDispatcherSingle dispatcher;
    T a("A");
    REQUIRE(a.value == "");
    dispatcher(a);
    REQUIRE(a.value == "a");
}

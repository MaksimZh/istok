// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <gui\dispatcher.hpp>

#include <memory>
#include <vector>

namespace {
    struct FakeArg {
        std::string id;
        std::string value;

        FakeArg(std::string id) : id(id) {}
    };

    class FakeDispatcher: public Dispatcher<FakeArg, std::string> {
    protected:
        std::string keyof(const FakeArg& arg) const override {
            return arg.id;
        }
        
        template<typename T>
        class FakeCaller: public Caller {
        public:
            FakeCaller(const std::string& id, MethodPtr<T, FakeArg> method)
            : id(id), method(method) {}

            std::string key() const override {
                return id;
            }

            virtual bool fits(const FakeArg& arg) const override {
                return arg.id.starts_with(id);
            }

            void operator()(Dispatcher& handler, FakeArg& arg) override {
                (static_cast<T*>(&handler)->*method)(arg);
            }
        
        private:
            std::string id;
            MethodPtr<T, FakeArg> method;
        };


    template <typename... Args>
    void init(Args... args) {
        if (initialized()) {
            nextInit();
            return;
        }
        std::vector<std::unique_ptr<Caller>> callers;
        packRec(callers, args...);
        firstInit(std::move(callers));
    }

    private:
        template <typename T, typename... Args>
        static void packRec(
                std::vector<std::unique_ptr<Caller>>& callers,
                const std::string& key, MethodPtr<T, FakeArg> method,
                Args... args) {
            callers.push_back(std::make_unique<FakeCaller<T>>(key, method));
            packRec(callers, args...);
        }

        static void packRec(std::vector<std::unique_ptr<Caller>>& callers) {}
    };

    
    class FakeDispatcherFlat: public FakeDispatcher {
    public:
        FakeDispatcherFlat() {
            init(
                "A", &FakeDispatcherFlat::processA,
                "B", &FakeDispatcherFlat::processB);
        }

        void processA(FakeArg& arg) {
            arg.value = "a";
        }

        void processB(FakeArg& arg) {
            arg.value = "b";
        }
    };


    class FakeDispatcherFallback: public FakeDispatcher {
    public:
        FakeDispatcherFallback() {
            init(
                "A", &FakeDispatcherFallback::processA,
                "AA", &FakeDispatcherFallback::processAA);
        }

        void processA(FakeArg& arg) {
            arg.value = "a";
        }

        void processAA(FakeArg& arg) {
            arg.value = "aa";
        }
    };


    class FakeDispatcherComplex: public FakeDispatcher {
    public:
        FakeDispatcherComplex() {
            init(
                "AA", &FakeDispatcherComplex::processAA,
                "A", &FakeDispatcherComplex::processA,
                "AAA", &FakeDispatcherComplex::processAAA,
                "ABB", &FakeDispatcherComplex::processABB);
        }

        void processA(FakeArg& arg) {
            arg.value = "a";
        }

        void processAA(FakeArg& arg) {
            arg.value = "aa";
        }

        void processAAA(FakeArg& arg) {
            arg.value = "aaa";
        }

        void processABB(FakeArg& arg) {
            arg.value = "abb";
        }
    };
}


TEST_CASE("Dispatcher - flat", "[unit][gui]") {
    FakeDispatcherFlat handler;
    FakeArg a("A");
    FakeArg b("B");
    REQUIRE(a.value == "");
    REQUIRE(b.value == "");
    handler(a);
    handler(b);
    REQUIRE(a.value == "a");
    REQUIRE(b.value == "b");
}


TEST_CASE("Dispatcher - fallback", "[unit][gui]") {
    FakeDispatcherFallback handler;
    FakeArg a("A");
    FakeArg aa("AA");
    FakeArg ab("AB");
    REQUIRE(a.value == "");
    REQUIRE(aa.value == "");
    REQUIRE(ab.value == "");
    handler(a);
    handler(aa);
    handler(ab);
    REQUIRE(a.value == "a");
    REQUIRE(aa.value == "aa");
    REQUIRE(ab.value == "a");
}


TEST_CASE("Dispatcher - complex", "[unit][gui]") {
    FakeDispatcherComplex handler;
    FakeArg a("A");
    FakeArg aa("AA");
    FakeArg ab("AB");
    FakeArg aaa("AAA");
    FakeArg aab("AAB");
    FakeArg aba("ABA");
    FakeArg abb("ABB");
    REQUIRE(a.value == "");
    REQUIRE(aa.value == "");
    REQUIRE(ab.value == "");
    REQUIRE(aaa.value == "");
    REQUIRE(aab.value == "");
    REQUIRE(aba.value == "");
    REQUIRE(abb.value == "");
    handler(a);
    handler(aa);
    handler(ab);
    handler(aaa);
    handler(aab);
    handler(aba);
    handler(abb);
    REQUIRE(a.value == "a");
    REQUIRE(aa.value == "aa");
    REQUIRE(ab.value == "a");
    REQUIRE(aaa.value == "aaa");
    REQUIRE(aab.value == "aa");
    REQUIRE(aba.value == "a");
    REQUIRE(abb.value == "abb");
}

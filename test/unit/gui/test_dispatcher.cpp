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
    };

    
    class FakeDispatcherFlat: public FakeDispatcher {
    public:
        FakeDispatcherFlat() {
            init();
        }

        std::vector<std::unique_ptr<Caller>> getCallers() {
            std::vector<std::unique_ptr<Caller>> result;
            result.push_back(std::make_unique<FakeCaller<FakeDispatcherFlat>>(
                "A", &FakeDispatcherFlat::processA));
            result.push_back(std::make_unique<FakeCaller<FakeDispatcherFlat>>(
                "B", &FakeDispatcherFlat::processB));
            return result;
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
            init();
        }

        std::vector<std::unique_ptr<Caller>> getCallers() {
            std::vector<std::unique_ptr<Caller>> result;
            result.push_back(std::make_unique<FakeCaller<FakeDispatcherFallback>>(
                "A", &FakeDispatcherFallback::processA));
            result.push_back(std::make_unique<FakeCaller<FakeDispatcherFallback>>(
                "AA", &FakeDispatcherFallback::processAA));
            return result;
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
            init();
        }

        std::vector<std::unique_ptr<Caller>> getCallers() {
            std::vector<std::unique_ptr<Caller>> result;
            result.push_back(std::make_unique<FakeCaller<FakeDispatcherComplex>>(
                "AA", &FakeDispatcherComplex::processAA));
            result.push_back(std::make_unique<FakeCaller<FakeDispatcherComplex>>(
                "A", &FakeDispatcherComplex::processA));
            result.push_back(std::make_unique<FakeCaller<FakeDispatcherComplex>>(
                "AAA", &FakeDispatcherComplex::processAAA));
            result.push_back(std::make_unique<FakeCaller<FakeDispatcherComplex>>(
                "ABB", &FakeDispatcherComplex::processABB));
            return result;
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

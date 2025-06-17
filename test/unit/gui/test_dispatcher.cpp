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

    class FakeHandler: public DispatchedHandler<FakeArg, std::string> {
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
                return false;
            }

            void operator()(DispatchedHandler& handler, FakeArg& arg) override {
                (static_cast<T*>(&handler)->*method)(arg);
            }

        
        private:
            std::string id;
            MethodPtr<T, FakeArg> method;
        };
    };

    class FakeHandlerSimple: public FakeHandler {
    public:
        FakeHandlerSimple() {
            init();
        }

        std::vector<std::unique_ptr<Caller>> getCallers() {
            std::vector<std::unique_ptr<Caller>> result;
            result.push_back(std::make_unique<FakeCaller<FakeHandlerSimple>>(
                "A", &FakeHandlerSimple::processA));
            result.push_back(std::make_unique<FakeCaller<FakeHandlerSimple>>(
                "B", &FakeHandlerSimple::processB));
            return result;
        }

        void processA(FakeArg& arg) {
            arg.value = "a";
        }

        void processB(FakeArg& arg) {
            arg.value = "b";
        }
    };
}


TEST_CASE("Simple dispatcher - simple", "[unit][gui]") {
    FakeHandlerSimple handler;
    FakeArg a("A");
    FakeArg b("B");
    REQUIRE(a.value == "");
    REQUIRE(b.value == "");
    handler(a);
    handler(b);
    REQUIRE(a.value == "a");
    REQUIRE(b.value == "b");
}

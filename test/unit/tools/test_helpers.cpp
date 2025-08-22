// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#include <catch.hpp>

#include <tools/helpers.hpp>

using namespace Istok::Tools;

#include <type_traits>

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

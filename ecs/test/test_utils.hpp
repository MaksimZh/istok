// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <memory>

#include <catch.hpp>
#include <catch2/trompeloeil.hpp>

#include "istok/ecs/system.hpp"

namespace Istok::ECS {

struct DeathWatch {
    Closure callback_;
    DeathWatch(Closure&& callback) : callback_(std::move(callback)) {}
    ~DeathWatch() { callback_(); }
};

struct MockValue {
    using Type = std::unique_ptr<DeathWatch>;
    MAKE_MOCK0(kill, void(), noexcept);

    Type get() {
        return std::make_unique<DeathWatch>([this]() noexcept { kill(); });
    }
};

struct MockClosure {
    MAKE_MOCK0(run, void(), noexcept);
    MAKE_MOCK0(kill, void(), noexcept);

    Closure get() {
        auto dw = std::make_unique<DeathWatch>([this]() noexcept { kill(); });
        return [this, x=std::move(dw)]() noexcept { run(); };
    }
};

}  // namespace Istok::ECS

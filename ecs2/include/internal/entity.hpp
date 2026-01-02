// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <cstdint>
#include <functional>


namespace Istok::ECS {

struct Entity {
    struct Hasher {
        size_t operator()(const Entity& entity) const {
            return std::hash<uint64_t>()(entity.value_);
        }
    };

    constexpr Entity() : value_(0) {}
    constexpr Entity(size_t index, size_t generation)
        : value_(index + (generation << 32)) {}
    bool operator==(const Entity& other) const = default;

    uint32_t index() const {
        return value_ & LOWER_MASK;
    }

    uint32_t generation() const {
        return (value_ & UPPER_MASK) >> 32;
    }

private:
    static constexpr uint64_t LOWER_MASK = 0x00000000ffffffff;
    static constexpr uint64_t UPPER_MASK = 0xffffffff00000000;
    uint64_t value_;
};

}  // namespace Istok::ECS

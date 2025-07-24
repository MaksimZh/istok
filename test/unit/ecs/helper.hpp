// helper.hpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <ecs/entity.hpp>
#include <unordered_set>

namespace {
    using EntityUSet = std::unordered_set<
        Istok::ECS::Entity,
        Istok::ECS::Entity::Hasher>;

    template <std::ranges::input_range R>
    bool isSameEntitySet(const R& x, const EntityUSet& y) {
        std::vector<Istok::ECS::Entity> xv(std::ranges::begin(x), std::ranges::end(x));
        EntityUSet xs(xv.begin(), xv.end());
        bool allUnique = (xv.size() == xs.size());
        return allUnique && xs == y;
    }
}

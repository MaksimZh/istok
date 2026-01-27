// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "internal/entity.hpp"
#include "internal/component.hpp"
#include <cassert>
#include <functional>
#include <utility>
#include <vector>


namespace Istok::ECS {

class ECSManager;
using System = std::function<void(ECSManager&)>;

class ECSManager {
public:
    bool isValidEntity(Entity entity) const noexcept {
        return entityManager_.isValidEntity(entity);
    }
    
    Entity createEntity() noexcept {
        return entityManager_.createEntity();
    }

    void deleteEntity(Entity entity) noexcept {
        assert(isValidEntity(entity));
        componentManager_.clearIndex(entity.index());
        entityManager_.deleteEntity(entity);
    }

    template <typename Component>
    bool has(Entity entity) const noexcept {
        assert(isValidEntity(entity));
        return componentManager_.has<Component>(entity.index());
    }

    template <typename Component>
    void insert(Entity entity, Component&& component) noexcept {
        assert(isValidEntity(entity));
        componentManager_.insert(
            entity.index(), std::forward<Component>(component));
    }

    template <typename Component>
    Component& get(Entity entity) noexcept {
        assert(isValidEntity(entity));
        assert(has<Component>(entity));
        return componentManager_.get<Component>(entity.index());
    }

    template <typename Component>
    void remove(Entity entity) noexcept {
        assert(isValidEntity(entity));
        assert(has<Component>(entity));
        componentManager_.remove<Component>(entity.index());
    }

    template <typename Component>
    void removeAll() noexcept {
        componentManager_.removeAll<Component>();
    }

    template<typename... Components>
    auto view() {
        return componentManager_.view<Components...>();
    }

    void addLoopSystem(System system) {
        loopSystems_.push_back(system);
    }

    void iterate() {
        for (auto& s : loopSystems_) {
            s(*this);
        }
    }

private:
    Internal::EntityManager entityManager_;
    Internal::ComponentManager componentManager_;
    std::vector<System> loopSystems_;
};

}  // namespace Istok::ECS

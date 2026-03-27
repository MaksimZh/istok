// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <cassert>
#include <ranges>

#include "ecs/component.hpp"
#include "ecs/entity.hpp"
#include "ecs/system.hpp"

namespace Istok::ECS {

class ECSManager {
public:
    ECSManager() = default;

    ~ECSManager() {
        systemManager_.clear();
        componentManager_.clear();
    }

    ECSManager(const ECSManager&) = delete;
    ECSManager& operator=(const ECSManager&) = delete;
    ECSManager(ECSManager&&) = default;
    ECSManager& operator=(ECSManager&&) = default;

    bool isValidEntity(Entity entity) const noexcept {
        return entityManager_.isValid(entity);
    }

    Entity createEntity() noexcept {
        return entityManager_.create();
    }

    void removeEntity(Entity entity) noexcept {
        assert(isValidEntity(entity));
        componentManager_.clearIndex(entity.index());
        entityManager_.remove(entity);
    }

    template <typename Component>
    bool has(Entity entity) const noexcept {
        assert(isValidEntity(entity));
        return componentManager_.has<Component>(entity.index());
    }

    template <typename Component>
    size_t count() const noexcept {
        return componentManager_.count<Component>();
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
    auto view() noexcept {
        return componentManager_.view<Components...>()
            | std::ranges::views::transform(
                [em=&entityManager_](size_t index) { return em->get(index); });
    }

    void addLoopSystem(Closure&& system) noexcept {
        systemManager_.addLoop(std::move(system));
    }

    void addHeadCleanupSystem(Closure&& system) noexcept {
        systemManager_.addHead(std::move(system));
    }

    void addTailCleanupSystem(Closure&& system) noexcept {
        systemManager_.addTail(std::move(system));
    }

    void iterate() noexcept {
        systemManager_.iterate();
    }

    void pass() noexcept {
        systemManager_.pass();
    }

private:
    Internal::EntityManager entityManager_;
    Internal::ComponentManager componentManager_;
    Internal::SystemManager systemManager_;
};

}  // namespace Istok::ECS

// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <cassert>
#include <functional>
#include <queue>
#include <ranges>
#include <stack>
#include <vector>

#include "ecs/component.hpp"
#include "ecs/entity.hpp"
#include "ecs/system.hpp"

namespace Istok::ECS {

class ECSManager;
using System = std::move_only_function<void() noexcept>;

class ECSManager {
public:
    ECSManager() noexcept
    : loopSystems_(std::make_unique<Internal::ClosureLoop>()) {};

    ~ECSManager() {
        loopSystems_.reset();
        while (!headCleanupSystems_.empty()) {
            headCleanupSystems_.front()();
            headCleanupSystems_.pop();
        }
        while (!tailCleanupSystems_.empty()) {
            tailCleanupSystems_.top()();
            tailCleanupSystems_.pop();
        }
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

    // TODO: removeEntity
    void deleteEntity(Entity entity) noexcept {
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

    void addLoopSystem(System&& system) noexcept {
        loopSystems_->add(std::move(system));
    }

    void addHeadCleanupSystem(System&& system) noexcept {
        headCleanupSystems_.push(std::move(system));
    }

    void addTailCleanupSystem(System&& system) noexcept {
        tailCleanupSystems_.push(std::move(system));
    }

    void iterate() noexcept {
        loopSystems_->iterate();
    }

    void pass() noexcept {
        loopSystems_->pass();
    }

private:
    Internal::EntityManager entityManager_;
    Internal::ComponentManager componentManager_;

    // TODO: Extract into SystemManager class
    std::unique_ptr<Internal::ClosureLoop> loopSystems_;
    std::queue<System> headCleanupSystems_;  // TODO: Extract into CleanupQueue class
    std::stack<System> tailCleanupSystems_;  // TODO: Extract into CleanupStack class
};

}  // namespace Istok::ECS

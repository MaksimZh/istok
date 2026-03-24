// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <cassert>
#include <functional>
#include <queue>
#include <ranges>
#include <stack>
#include <vector>

#include "ecs/entity.hpp"
#include "ecs/component.hpp"

namespace Istok::ECS {

class ECSManager;
using System = std::move_only_function<void(ECSManager&) noexcept>;

class ECSManager {
public:
    ECSManager() = default;

    ~ECSManager() {
        while (!loopSystems_.empty()) {
            loopSystems_.pop_back();
        }
        while (!headCleanupSystems_.empty()) {
            headCleanupSystems_.front()(*this);
            headCleanupSystems_.pop();
        }
        while (!tailCleanupSystems_.empty()) {
            tailCleanupSystems_.top()(*this);
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
        loopSystems_.push_back(std::move(system));
        loopRunFlags_.push_back(false);
    }

    void addHeadCleanupSystem(System&& system) noexcept {
        headCleanupSystems_.push(std::move(system));
    }

    void addTailCleanupSystem(System&& system) noexcept {
        tailCleanupSystems_.push(std::move(system));
    }

    // Run all loop systems in order.
    // If called from a loop system, run all loop systems in order except the
    // current one making closed loop starting right after the current one.
    // Iterations can be nested until all loop systems exhausted.
    void iterate() noexcept {
        int sentinel = currentLoopSystem_;
        for (
            currentLoopSystem_ = sentinel + 1;
            currentLoopSystem_ < loopSystems_.size();
            ++currentLoopSystem_
        ) {
            if (loopRunFlags_[currentLoopSystem_]) {
                continue;
            }
            loopRunFlags_[currentLoopSystem_] = true;
            loopSystems_[currentLoopSystem_](*this);
            loopRunFlags_[currentLoopSystem_] = false;
        }
        for (
            currentLoopSystem_ = 0;
            currentLoopSystem_ < sentinel;
            ++currentLoopSystem_
        ) {
            if (loopRunFlags_[currentLoopSystem_]) {
                continue;
            }
            loopRunFlags_[currentLoopSystem_] = true;
            loopSystems_[currentLoopSystem_](*this);
            loopRunFlags_[currentLoopSystem_] = false;
        }
        currentLoopSystem_ = sentinel;
    }

private:
    Internal::EntityManager entityManager_;
    Internal::ComponentManager componentManager_;
    std::vector<System> loopSystems_;
    std::queue<System> headCleanupSystems_;
    std::stack<System> tailCleanupSystems_;
    std::vector<bool> loopRunFlags_;
    int currentLoopSystem_ = -1;
};

}  // namespace Istok::ECS

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
    }

    void addHeadCleanupSystem(System&& system) noexcept {
        headCleanupSystems_.push(std::move(system));
    }

    void addTailCleanupSystem(System&& system) noexcept {
        tailCleanupSystems_.push(std::move(system));
    }

    void iterate() noexcept {
        if (currentLoopSystem_ != kIdleFlag) {
            return;
        }
        for (
            currentLoopSystem_ = 0;
            currentLoopSystem_ < loopSystems_.size();
            ++currentLoopSystem_
        ) {
            loopSystems_[currentLoopSystem_](*this);
        }
        currentLoopSystem_ = -1;
    }

    void pass() noexcept {
        if (currentLoopSystem_ < 0) {
            return;
        }
        size_t sentinel = currentLoopSystem_;
        currentLoopSystem_ = kPassFlag;
        for (size_t i = sentinel + 1; i < loopSystems_.size(); ++i) {
            loopSystems_[i](*this);
        }
        for (size_t i = 0; i < sentinel; ++i) {
            loopSystems_[i](*this);
        }
        currentLoopSystem_ = sentinel;
    }

private:
    constexpr static int kIdleFlag = -1;
    constexpr static int kPassFlag = -2;
    Internal::EntityManager entityManager_;
    Internal::ComponentManager componentManager_;
    std::vector<System> loopSystems_;
    std::queue<System> headCleanupSystems_;
    std::stack<System> tailCleanupSystems_;
    int currentLoopSystem_ = kIdleFlag;
};

}  // namespace Istok::ECS

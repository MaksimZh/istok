// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <cassert>
#include <functional>
#include <queue>
#include <stack>
#include <vector>

#include "ecs/entity.hpp"
#include "ecs/component.hpp"

namespace Istok::ECS {

class ECSManager;
using System = std::move_only_function<void(ECSManager&) noexcept>;

namespace Internal {

template <typename IndexView>
class EntityView {
public:
    class Iterator {
    public:
        using element_type = size_t;
        using difference_type = ptrdiff_t;

        Iterator(const EntityManager& entityManager, IndexView::iterator index)
        : entityManager_(&entityManager), index_(index) {}

        Entity operator*() const noexcept {
            return entityManager_->entityFromIndex(*index_);
        }

        Iterator& operator++() noexcept {
            ++index_;
            return *this;
        }

        Iterator operator++(int) noexcept {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const Iterator& other) const noexcept {
            return index_ == other.index_;
        }

    private:
        const EntityManager* entityManager_;
        IndexView::iterator index_;
    };

    using iterator = Iterator;

    iterator begin() noexcept {
        return Iterator(*entityManager_, view_.begin());
    }

    iterator end() noexcept {
        return Iterator(*entityManager_, view_.end());
    }

    using const_iterator = Iterator;

    const_iterator begin() const noexcept {
        return Iterator(*entityManager_, view_.begin());
    }

    const_iterator end() const noexcept {
        return Iterator(*entityManager_, view_.end());
    }

    EntityView(
        const EntityManager& entityManager,
        IndexView view
    ) : entityManager_(&entityManager), view_(view) {}

    EntityView(const EntityView&) = delete;
    EntityView& operator=(const EntityView&) = delete;
    EntityView(EntityView&&) = delete;
    EntityView& operator=(EntityView&&) = delete;

private:
    const EntityManager* entityManager_;
    const IndexView view_;
};

}  // namespace Internal


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

    size_t countEntities() const noexcept {
        return entityManager_.size();
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
        return Internal::EntityView(
            entityManager_,
            componentManager_.view<Components...>());
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
        for (auto& s : loopSystems_) {
            s(*this);
        }
    }

private:
    Internal::EntityManager entityManager_;
    Internal::ComponentManager componentManager_;
    std::vector<System> loopSystems_;
    std::queue<System> headCleanupSystems_;
    std::stack<System> tailCleanupSystems_;
};

}  // namespace Istok::ECS

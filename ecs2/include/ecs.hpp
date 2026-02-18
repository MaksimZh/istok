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
    ~ECSManager() {
        for (
            auto it = cleanupSystems_.rbegin();
            it != cleanupSystems_.rend();
            ++it
        ) {
            (*it)(*this);
        }
    }

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

    void addLoopSystem(System system) {
        loopSystems_.push_back(system);
    }

    void addCleanupSystem(System system) {
        cleanupSystems_.push_back(system);
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
    std::vector<System> cleanupSystems_;
};

}  // namespace Istok::ECS

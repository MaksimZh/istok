// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "internal/entity.hpp"
#include "internal/component.hpp"
#include <cassert>
#include <utility>


namespace Istok::ECS {

class ECSManager {
public:
    bool isValidEntity(Entity entity) const noexcept {
        return entityManager.isValidEntity(entity);
    }
    
    Entity createEntity() noexcept {
        return entityManager.createEntity();
    }

    void deleteEntity(Entity entity) noexcept {
        assert(isValidEntity(entity));
        entityManager.deleteEntity(entity);
    }

    template <typename Component>
    bool has(Entity entity) const noexcept {
        assert(isValidEntity(entity));
        return componentManager.has<Component>(entity.index());
    }

    template <typename Component>
    void insert(Entity entity, Component&& component) noexcept {
        assert(isValidEntity(entity));
        componentManager.insert(
            entity.index(), std::forward<Component>(component));
    }

    template <typename Component>
    Component& get(Entity entity) noexcept {
        assert(isValidEntity(entity));
        assert(has<Component>(entity));
        return componentManager.get<Component>(entity.index());
    }

    template <typename Component>
    void remove(Entity entity) noexcept {
        assert(isValidEntity(entity));
        assert(has<Component>(entity));
        componentManager.remove<Component>(entity.index());
    }

    template <typename Component>
    void removeAll() noexcept {
        componentManager.removeAll<Component>();
    }

private:
    Internal::EntityManager entityManager;
    Internal::ComponentManager componentManager;
};

}  // namespace Istok::ECS

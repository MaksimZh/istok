// manager.hpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "manager.hpp"

#include <cassert>

namespace Istok::ECS {

class BoundEntity {
public:
    BoundEntity(EntityComponentManager& manager, Entity entity)
    : manager(&manager), entity(entity) {}
    
    BoundEntity(const BoundEntity&) = default;
    BoundEntity& operator=(const BoundEntity&) = default;
    BoundEntity(BoundEntity&&) = default;
    BoundEntity& operator=(BoundEntity&&) = default;

    EntityComponentManager& getManager() const {
        return *manager;
    }

    Entity getEntity() const {
        return entity;
    }

    bool isValid() const {
        return manager->isValidEntity(entity);
    }

    template <typename Component>
    bool has() const {
        assert(isValid());
        return manager->has<Component>(entity);
    }

    void destroy() {
        assert(isValid());
        manager->destroyEntity(entity);
    }

    template <typename Component>
    void set(Component&& component) {
        assert(isValid());
        manager->set<Component>(entity, std::move(component));
    }

    template <typename Component>
    Component& get() {
        assert(isValid());
        assert(has<Component>());
        return manager->get<Component>(entity);
    }

    template <typename Component>
    const Component& get(Entity e) const {
        assert(isValid());
        assert(has<Component>());
        return manager->get<Component>(entity);
    }

    template <typename Component>
    void remove() {
        assert(isValid());
        assert(has<Component>());
        manager->remove<Component>(entity);
    }

private:
    EntityComponentManager* manager;
    Entity entity;
};

} // namespace Istok::ECS

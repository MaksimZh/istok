// manager.hpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "datastruct.hpp"
#include "entity.hpp"
#include "component.hpp"

namespace Istok::ECS {

class EntityComponentManager {
public:
    EntityComponentManager(const EntityComponentManager&) = delete;
    EntityComponentManager& operator=(const EntityComponentManager&) = delete;
    EntityComponentManager(EntityComponentManager&&) = default;
    EntityComponentManager& operator=(EntityComponentManager&&) = default;

    EntityComponentManager(size_t initialCapacity)
        : entities(initialCapacity) {}

    Entity createEntity() {
        return entities.create();
    }

    bool isValidEntity(Entity e) const {
        return entities.isValid(e);
    }

    template <typename Component>
    bool hasComponent(Entity e) const {
        return components.has<Component>(e);
    }

    template <typename Component>
    void addComponent(Entity e, Component&& component) {
        components.add(e, std::move(component));
    }

    template <typename Component>
    void removeComponent(Entity e) {
        components.remove<Component>(e);
    }

    void destroyEntity(Entity e) {
        entities.destroy(e);
    }

private:
    EntityManager entities;
    ComponentManager components;
};

} // namespace Istok::ECS

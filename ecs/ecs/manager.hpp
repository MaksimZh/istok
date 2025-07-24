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

    bool isValidEntity(Entity e) const {
        return entities.isValid(e);
    }

    template <typename Component>
    bool hasComponent(Entity e) const {
        return components.has<Component>(e);
    }

    template <typename Component>
    Component& getComponent(Entity e) {
        assert(hasComponent<Component>(e));
        return components.get<Component>(e);
    }

    template <typename Component>
    const Component& getComponent(Entity e) const {
        return components.get<Component>(e);
    }

    Entity createEntity() {
        return entities.create();
    }

    void destroyEntity(Entity e) {
        assert(isValidEntity(e));
        components.clean(e);
        entities.destroy(e);
    }

    template <typename Component>
    void addComponent(Entity e, Component&& component) {
        components.add(e, std::move(component));
    }

    template <typename Component>
    void removeComponent(Entity e) {
        components.remove<Component>(e);
    }

    template<typename... Components>
    EntityStorageRange view() {
        return components.view<Components...>();
    }

private:
    EntityManager entities;
    ComponentManager components;
};

} // namespace Istok::ECS

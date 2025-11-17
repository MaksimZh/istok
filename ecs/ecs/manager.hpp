// manager.hpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "datastruct.hpp"
#include "entity.hpp"
#include "component.hpp"

#include <cassert>

namespace Istok::ECS {

class EntityComponentManager {
public:
    EntityComponentManager(const EntityComponentManager&) = delete;
    EntityComponentManager& operator=(const EntityComponentManager&) = delete;
    EntityComponentManager(EntityComponentManager&&) = default;
    EntityComponentManager& operator=(EntityComponentManager&&) = default;

    EntityComponentManager(size_t initialCapacity = 1024)
        : entities(initialCapacity) {}
    
    void clear() {
        components.clear();
    }

    bool isValidEntity(Entity e) const {
        return entities.isValid(e);
    }

    template <typename Component>
    bool has(Entity e) const {
        assert(isValidEntity(e));
        return components.has<Component>(e);
    }

    template <typename Component>
    Component& get(Entity e) {
        assert(isValidEntity(e));
        assert(has<Component>(e));
        return components.get<Component>(e);
    }

    template <typename Component>
    const Component& get(Entity e) const {
        assert(isValidEntity(e));
        assert(has<Component>(e));
        return components.get<Component>(e);
    }

    Entity createEntity() {
        return entities.create();
    }

    void destroyEntity(Entity e) {
        assert(isValidEntity(e));
        components.clear(e);
        entities.destroy(e);
    }

    template <typename Component>
    void set(Entity e, Component&& component) {
        assert(isValidEntity(e));
        components.insert(e, std::move(component));
    }

    template <typename Component>
    void remove(Entity e) {
        assert(isValidEntity(e));
        assert(has<Component>(e));
        components.remove<Component>(e);
    }

    template <typename Component>
    void removeAll() {
        components.removeAll<Component>();
    }

    template<typename... Components>
    EntityView view() {
        return components.view<Components...>();
    }

private:
    EntityManager entities;
    ComponentManager components;
};

} // namespace Istok::ECS

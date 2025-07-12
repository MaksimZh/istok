// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "entity.hpp"

#include <cassert>
#include <unordered_map>
#include <typeindex>
#include <memory>


class ComponentStorage {};

template <typename Component>
class ComponentStorageOf: public ComponentStorage {
public:
    bool has(Entity e) const {
        return data.contains(e);
    }
    
    void insert(Entity e, Component&& component) {
        data[e] = component;
    }

    Component& get(Entity e) {
        assert(has(e));
        return data[e];
    }

private:
    std::unordered_map<Entity, Component, Entity::Hash> data;
};


class ComponentManager {
public:
    ComponentManager() {}

    template<typename Component>
    bool has(Entity e) const {
        static std::type_index index(typeid(Component));
        auto it = storages.find(index);
        if (it == storages.end()) {
            return false;
        }
        auto& storage = *static_cast<ComponentStorageOf<Component>*>(it->second.get());
        return storage.has(e);
    }

    template<typename Component>
    void add(Entity e, Component&& component) {
        static std::type_index index(typeid(Component));
        auto it = storages.find(index);
        if (it == storages.end()) {
            auto result = storages.emplace(
                index,
                std::make_unique<ComponentStorageOf<Component>>());
            assert(result.second == true);
            it = result.first;
        }
        auto& storage = *static_cast<ComponentStorageOf<Component>*>(it->second.get());
        storage.insert(e, std::move(component));
    }

    template<typename Component>
    Component& get(Entity e) {
        assert(has<Component>(e));
        static std::type_index index(typeid(Component));
        auto& storage = *static_cast<ComponentStorageOf<Component>*>(storages[index].get());
        return storage.get(e);
    }

private:
    std::unordered_map<
        std::type_index,
        std::unique_ptr<ComponentStorage>
    > storages;
};

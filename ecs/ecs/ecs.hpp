// ecs.hpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "entity.hpp"

#include <cassert>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <vector>
#include <ranges>

namespace Istok::ECS {

class ComponentStorage {
public:
    ComponentStorage() = default;
    ComponentStorage(const ComponentStorage&) = delete;
    ComponentStorage& operator=(const ComponentStorage&) = delete;
    ComponentStorage(ComponentStorage&&) noexcept = default;
    ComponentStorage& operator=(ComponentStorage&&) noexcept = default;
    
    virtual size_t size() const = 0;
    virtual bool has(Entity e) const = 0;
    virtual void remove(Entity e) = 0;
    virtual DenseRangeX<Entity> byEntity() = 0;
};


template <typename Component>
class ComponentStorageOf: public ComponentStorage {
public:
    ComponentStorageOf() = default;
    ComponentStorageOf(const ComponentStorageOf&) = delete;
    ComponentStorageOf& operator=(const ComponentStorageOf&) = delete;
    ComponentStorageOf(ComponentStorageOf&&) noexcept = default;
    ComponentStorageOf& operator=(ComponentStorageOf&&) noexcept = default;
    
    size_t size() const override {
        return container.size();
    }    
    
    bool has(Entity e) const override {
        return container.contains(e);
    }
    
    void insert(Entity e, Component&& component) {
        container.insert(e, component);
    }

    Component& get(Entity e) {
        assert(has(e));
        return container.get(e);
    }

    void remove(Entity e) override {
        assert(has(e));
        container.erase(e);
    }

    DenseRangeX<Entity> byEntity() override {
        return container.byKey();
    }

private:
    DenseMap<Entity, Component, Entity::Hasher> container;
};


class ComponentStorageManager {
public:
    ComponentStorageManager() = default;
    ComponentStorageManager(const ComponentStorageManager&) = delete;
    ComponentStorageManager& operator=(const ComponentStorageManager&) = delete;
    ComponentStorageManager(ComponentStorageManager&&) noexcept = default;
    ComponentStorageManager& operator=(ComponentStorageManager&&) noexcept = default;

    template<typename Component>
    bool hasStorage() const {
        return storages.contains(key<Component>());
    }

    template <typename Component>
    const ComponentStorageOf<Component>& getStorage() const {
        auto it = storages.find(key<Component>());
        assert(it != storages.end());
        return *static_cast<ComponentStorageOf<Component>*>(it->second.get());
    }

    template <typename Component>
    ComponentStorageOf<Component>& getStorage() {
        return const_cast<ComponentStorageOf<Component>&>(
            static_cast<const ComponentStorageManager*>(this)
                -> getStorage<Component>());
    }

    template <typename Component>
    ComponentStorageOf<Component>& getOrCreateStorage() {
        auto it = storages.find(key<Component>());
        if (it == storages.end()) {
            auto result = storages.emplace(
                key<Component>(),
                std::make_unique<ComponentStorageOf<Component>>());
            assert(result.second == true);
            it = result.first;
        }
        return *static_cast<ComponentStorageOf<Component>*>(it->second.get());
    }

    auto byStorage() {
        return storages
            | std::views::values
            | std::views::transform(
                [](auto& ptr) -> ComponentStorage& { return *ptr; });
    }

private:
    std::unordered_map<
        std::type_index,
        std::unique_ptr<ComponentStorage>
    > storages;

    template <typename Component>
    constexpr std::type_index key() const {
        static std::type_index index(typeid(Component));
        return index;
    }
};


class EntityStorageFilter {
public:
    template <std::ranges::forward_range R>
    requires std::convertible_to<
        std::ranges::range_reference_t<R>, ComponentStorage&>
    explicit EntityStorageFilter(R&& storages)
        : storages(std::ranges::begin(storages), std::ranges::end(storages)) {}

    bool operator()(Entity e) const {
        for (const auto& s : storages) {
            if (!s.get().has(e)) {
                return false;
            }
        }
        return true;
    }

private:
    std::vector<std::reference_wrapper<ComponentStorage>> storages;
};


class ComponentManager {
public:
    ComponentManager() = default;
    ComponentManager(const ComponentManager&) = delete;
    ComponentManager& operator=(const ComponentManager&) = delete;
    ComponentManager(ComponentManager&&) noexcept = default;
    ComponentManager& operator=(ComponentManager&&) noexcept = default;

    template<typename Component>
    bool has(Entity e) const {
        return storages.hasStorage<Component>() &&
            storages.getStorage<Component>().has(e);
    }

    template<typename Component>
    void add(Entity e, Component&& component) {
        ComponentStorageOf<Component>& storage =
            storages.getOrCreateStorage<Component>();
        storage.insert(e, std::move(component));
    }

    template<typename Component>
    Component& get(Entity e) {
        assert(storages.hasStorage<Component>());
        ComponentStorageOf<Component>& storage = 
            storages.getStorage<Component>();
        assert(storage.has(e));
        return storage.get(e);
    }

    template<typename Component>
    void remove(Entity e) {
        assert(storages.hasStorage<Component>());
        ComponentStorageOf<Component>& storage = 
            storages.getStorage<Component>();
        assert(storage.has(e));
        storage.remove(e);
    }

    void clean(Entity e) {
        for (auto& s : storages.byStorage()) {
            if (s.has(e)) {
                s.remove(e);
            }
        }
    }


    template<typename... Components>
    auto view() {
        std::vector<std::reference_wrapper<ComponentStorage>> found{
            storages.getOrCreateStorage<Components>()...};
        assert(found.size() > 0);
        std::sort(
            found.begin(), found.end(),
            [](const ComponentStorage& a, const ComponentStorage& b) {
                return a.size() < b.size();
            });
        return found.at(0).get().byEntity()
            | std::views::filter(
                EntityStorageFilter(found | std::views::drop(1)));
    }


private:
    ComponentStorageManager storages;
};

} // namespace Istok::ECS

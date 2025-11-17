// component.hpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "datastruct.hpp"
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
    virtual ~ComponentStorage() = default;
    
    virtual size_t size() const = 0;
    virtual bool has(Entity e) const = 0;
    virtual void remove(Entity e) = 0;
    virtual void removeAll() = 0;
    virtual DenseRange<Entity> byEntity() = 0;
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
        container.insert(e, std::move(component));
    }

    Component& get(Entity e) {
        assert(has(e));
        return container.get(e);
    }

    void remove(Entity e) override {
        assert(has(e));
        container.erase(e);
    }

    void removeAll() override {
        container.clear();
    }

    DenseRange<Entity> byEntity() override {
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


class EntityStorageIterator {
public:
    using element_type = Entity;
    using difference_type = ptrdiff_t;

    EntityStorageIterator() = default;
    EntityStorageIterator(
        DenseSafeScanner<Entity> start,
        const ContainerFilter<ComponentStorage>& filter)
        : current(start), filter(&filter) {
            seek();
        }

    const Entity& operator*() const { return *current.get(); }

    EntityStorageIterator& operator++() {
        current.inc();
        seek();
        return *this;
    }

    EntityStorageIterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const EntityStorageIterator& other) const = default;

private:
    DenseSafeScanner<Entity> current;
    const ContainerFilter<ComponentStorage>* filter;

    void seek() {
        while (!current.finished() && !(*filter)(*current.get())) {
            current.inc();
        }
    }
};


class EntityStorageRange {
public:
    using iterator = EntityStorageIterator;
    using const_iterator = EntityStorageIterator;

    EntityStorageRange(DenseRange<Entity> base, ContainerFilter<ComponentStorage> filter)
        : base(base), filter(filter) {}
    
    iterator begin() noexcept {
        return EntityStorageIterator(
            DenseSafeScanner<Entity>(base.begin(), base.end()),
            filter);
    }
    
    iterator end() noexcept {
        return EntityStorageIterator(
            DenseSafeScanner<Entity>(base.end(), base.end()),
            filter);
    }

    const_iterator begin() const noexcept {
        return EntityStorageIterator(
            DenseSafeScanner<Entity>(base.begin(), base.end()),
            filter);
    }
    
    const_iterator end() const noexcept {
        return EntityStorageIterator(
            DenseSafeScanner<Entity>(base.end(), base.end()),
            filter);
    }

    template <std::ranges::forward_range RN>
    EntityStorageRange exclude(RN&& neg) const {
        return EntityStorageRange(base, filter.exclude(neg));
    }

private:
    DenseRange<Entity> base;
    ContainerFilter<ComponentStorage> filter;
};


class EntityView {
public:
    using iterator = EntityStorageIterator;
    using const_iterator = EntityStorageIterator;

    EntityView(ComponentStorageManager& storages, EntityStorageRange range)
        : storages(&storages), range(range) {}
    
    iterator begin() noexcept { return range.begin(); }
    iterator end() noexcept { return range.end(); }
    const_iterator begin() const noexcept { return range.begin(); }
    const_iterator end() const noexcept { return range.end(); }

    template<typename... Components>
    EntityView exclude() {
        std::vector<std::reference_wrapper<ComponentStorage>> found{
            storages->getOrCreateStorage<Components>()...};
        return EntityView(*storages, range.exclude(found));
    }

private:
    ComponentStorageManager* storages;
    EntityStorageRange range;
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
    void insert(Entity e, Component&& component) {
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

    template<typename Component>
    void removeAll() {
        if (!storages.hasStorage<Component>()) {
            return;
        }
        storages.getStorage<Component>().removeAll();
    }

    void clear(Entity e) {
        for (auto& s : storages.byStorage()) {
            if (s.has(e)) {
                s.remove(e);
            }
        }
    }

    void clearAll() {
        for (auto& s : storages.byStorage()) {
            s.removeAll();
        }
    }

    template<typename... Components>
    EntityView view() {
        std::vector<std::reference_wrapper<ComponentStorage>> found{
            storages.getOrCreateStorage<Components>()...};
        assert(found.size() > 0);
        return EntityView(
            storages,
            EntityStorageRange(
                found.at(0).get().byEntity(),
                ContainerFilter<ComponentStorage>(found | std::views::drop(1))));
    }


private:
    ComponentStorageManager storages;
};

} // namespace Istok::ECS

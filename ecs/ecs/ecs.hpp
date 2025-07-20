// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "entity.hpp"

#include <cassert>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <vector>

namespace Istok::ECS {

class ComponentStorage {
public:
    virtual bool has(Entity e) const = 0;
    virtual void remove(Entity e) = 0;
    virtual DenseRange<Entity> byEntity() = 0;
};


template <typename Component>
class ComponentStorageOf: public ComponentStorage {
public:
    ComponentStorageOf() {};
    ComponentStorageOf(const ComponentStorageOf&) = delete;
    ComponentStorageOf& operator=(const ComponentStorageOf&) = delete;
    
    size_t size() const {
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

    DenseRange<Entity> byEntity() override {
        return container.byKey();
    }

private:
    DenseMap<Entity, Component, Entity::Hasher> container;
};


class ComponentManager {
public:
    ComponentManager() {}

    template<typename Component>
    bool has(Entity e) const {
        const ComponentStorageOf<Component>* storage = viewStorage<Component>();
        if (storage == nullptr) {
            return false;
        }
        return storage->has(e);
    }

    template<typename Component>
    void add(Entity e, Component&& component) {
        ComponentStorageOf<Component>& storage = prepareStorage<Component>();
        storage.insert(e, std::move(component));
    }

    template<typename Component>
    Component& get(Entity e) {
        ComponentStorageOf<Component>& storage = prepareStorage<Component>();
        assert(storage.has(e));
        return storage.get(e);
    }

    template<typename Component>
    void remove(Entity e) {
        ComponentStorageOf<Component>& storage = prepareStorage<Component>();
        assert(storage.has(e));
        storage.remove(e);
    }

    void clean(Entity e) {
        for (const auto& [key, value] : storages) {
            if (value->has(e)) {
                value->remove(e);
            }
        }
    }

    /*
    class View {
    public:
        explicit View(std::vector<ComponentStorage::View> source)
            : storages(std::move(source)) {
            std::sort(
                storages.begin(), storages.end(),
                [](const ComponentStorage::View& a, const ComponentStorage::View& b) {
                    return a.size() < b.size();
                });
        }

        class Iterator {
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = Entity;

            Iterator() {}
        };

    private:
        std::vector<ComponentStorage::View> storages;
    };


    template<typename... Components>
    View getView() {
        return View(std::vector<ComponentStorage::View>{
            prepareStorage<Components>().getView()...});
    }
    */

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

    template <typename Component>
    ComponentStorageOf<Component>& prepareStorage() {
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

    template <typename Component>
    const ComponentStorageOf<Component>* viewStorage() const {
        auto it = storages.find(key<Component>());
        if (it == storages.end()) {
            return nullptr;
        }
        return static_cast<ComponentStorageOf<Component>*>(it->second.get());
    }
};

} // namespace Istok::ECS

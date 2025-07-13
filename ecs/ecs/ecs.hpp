// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "entity.hpp"

#include <cassert>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <vector>


class ComponentStorage {
public:
    virtual bool has(Entity e) const = 0;
    virtual void remove(Entity e) = 0;

    class View {
    public:
        View(std::vector<Entity>& target, size_t limit)
            : target(target), limit(limit) {}

        View(const View& other)
            : target(other.target), limit(other.limit) {}

        View& operator=(const View& other) {
            if (this == &other) {
                return *this;
            }
            target = other.target;
            limit = other.limit;
            return *this;
        }
        
        using iterator = std::vector<Entity>::iterator;
        using const_iterator = std::vector<Entity>::const_iterator;

        size_t size() const {
            return limit;
        }

        iterator begin() {
            return target.begin();
        }

        const_iterator begin() const {
            return target.begin();
        }

        iterator end() {
            return target.begin() + limit;
        }

        const_iterator end() const {
            return target.begin() + limit;
        }

    private:
        std::vector<Entity>& target;
        size_t limit;
    };
    
    virtual View getView() = 0;
};


template <typename Component>
class ComponentStorageOf: public ComponentStorage {
public:
    ComponentStorageOf() {};
    ComponentStorageOf(const ComponentStorageOf&) = delete;
    ComponentStorageOf& operator=(const ComponentStorageOf&) = delete;
    
    bool has(Entity e) const override {
        return indexMap.contains(e);
    }
    
    void insert(Entity e, Component&& component) {
        if (has(e)) {
            components[indexMap[e]] = component;
            return;
        }
        indexMap[e] = entities.size();
        entities.push_back(e);
        components.push_back(component);
    }

    Component& get(Entity e) {
        assert(has(e));
        return components[indexMap[e]];
    }

    void remove(Entity e) override {
        size_t index = indexMap[e];
        indexMap.erase(e);
        if (index == entities.size() - 1) {
            entities.pop_back();
            components.pop_back();
            return;
        }
        Entity last = entities.back();
        indexMap[last] = index;
        entities[index] = last;
        components[index] = components.back();
        entities.pop_back();
        components.pop_back();
    }

    View getView() override {
        return View(entities, entities.size());
    }

private:
    std::unordered_map<Entity, size_t, Entity::Hash> indexMap;
    std::vector<Entity> entities;
    std::vector<Component> components;
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

        /*
        class Iterator {
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = Entity;

            Iterator() {}
        };
        */

    private:
        std::vector<ComponentStorage::View> storages;
    };


    template<typename... Components>
    View getView() {
        return View(std::vector<ComponentStorage::View>{
            prepareStorage<Components>().getView()...});
    }


private:
    using StoragesType = std::unordered_map<
        std::type_index,
        std::unique_ptr<ComponentStorage>>;
    StoragesType storages;

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

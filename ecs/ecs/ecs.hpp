// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "entity.hpp"

#include <cassert>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <vector>


template <typename T>
class DenseVector {
public:
    using iterator = std::vector<T>::iterator;
    using const_iterator = std::vector<T>::const_iterator;
    iterator begin() { return data.begin(); }
    iterator end() { return data.end(); }
    const_iterator begin() const { return data.begin(); }
    const_iterator end() const { return data.end(); }

    size_t size() const {
        return data.size();
    }
    
    T& operator[](size_t index) {
        assert(index < size());
        return data[index];
    }

    T& back() {
        assert(size() > 0);
        return data.back();
    }

    const T& back() const {
        assert(size() > 0);
        return data.back();
    }

    const T& operator[](size_t index) const {
        assert(index < size());
        return data[index];
    }
    
    void push_back(const T& value) {
        data.push_back(value);
    }

    void push_back(T&& value) {
        data.push_back(value);
    }

    void remove(size_t index) {
        assert(index < size());
        if (index < size() - 1) {
            data[index] = data.back();
        }
        data.pop_back();
    }

private:
    std::vector<T> data;
};


class EntityIndexMap {
public:
    bool contains(Entity e) const {
        return data.contains(e);
    }

    size_t& operator[](Entity e) {
        return data.insert(std::make_pair(e, 0)).first->second;
    }

    size_t operator[](Entity e) const {
        auto it = data.find(e);
        assert(it != data.end());
        return it->second;
    }

    void remove(Entity e) {
        data.erase(e);
    }

private:
    std::unordered_map<Entity, size_t, Entity::Hash> data;
};


class ComponentStorage {
public:
    virtual bool has(Entity e) const = 0;
    virtual void remove(Entity e) = 0;

    class View {
    public:
        using iterator = std::vector<Entity>::iterator;
        using const_iterator = std::vector<Entity>::const_iterator;

        View(iterator start, iterator sentinel)
            : start(start), sentinel(sentinel) {}

        size_t size() const {
            return sentinel - start;
        }

        iterator begin() { return start; }
        iterator end() { return sentinel; }
        const_iterator begin() const { return start; }
        const_iterator end() const { return sentinel; }

    private:
        iterator start;
        iterator sentinel;
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
        indexMap.remove(e);
        Entity last = entities.back();
        if (last != e) {
            indexMap[last] = index;
        }
        entities.remove(index);
        components.remove(index);
    }

    View getView() override {
        return View(entities.begin(), entities.end());
    }

private:
    EntityIndexMap indexMap;
    DenseVector<Entity> entities;
    DenseVector<Component> components;
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

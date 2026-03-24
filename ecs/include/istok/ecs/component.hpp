// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <cassert>
#include <cstddef>
#include <vector>
#include <span>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <ranges>
#include <utility>

namespace Istok::ECS::Internal {

class AbstractComponentStorage {
public:
    virtual ~AbstractComponentStorage() = default;
    virtual void removeIfHas(size_t index) noexcept = 0;
};

template <typename T>
class ComponentStorage : public AbstractComponentStorage {
public:
    ComponentStorage() = default;
    ~ComponentStorage() = default;

    ComponentStorage(const ComponentStorage&) = delete;
    ComponentStorage& operator=(const ComponentStorage&) = delete;
    ComponentStorage(ComponentStorage&&) = default;
    ComponentStorage& operator=(ComponentStorage&&) = default;

    size_t size() const noexcept {
        return components_.size();
    }

    bool has(size_t index) const noexcept {
        return index < indexToComponent_.size()
            && indexToComponent_[index] >= 0;
    }

    T& get(size_t index) noexcept {
        assert(has(index));
        return components_[indexToComponent_[index]];
    }

    void insert(size_t index, T&& value) noexcept {
        if (has(index)) {
            components_[indexToComponent_[index]] = std::forward<T>(value);
            return;
        }
        if (index >= indexToComponent_.size()) {
            indexToComponent_.resize(index + 1, EMPTY);
        }
        indexToComponent_[index] = components_.size();
        components_.push_back(std::forward<T>(value));
        componentToIndex_.push_back(index);
    }

    void remove(size_t index) noexcept {
        assert(has(index));
        size_t componentIndex = indexToComponent_[index];
        if (componentIndex < components_.size() - 1) {
            indexToComponent_[componentToIndex_.back()] = componentIndex;
            components_[componentIndex] = std::move(components_.back());
            componentToIndex_[componentIndex] = componentToIndex_.back();
        }
        indexToComponent_[index] = EMPTY;
        components_.pop_back();
        componentToIndex_.pop_back();
    }

    void clear() noexcept {
        indexToComponent_.clear();
        components_.clear();
        componentToIndex_.clear();
    }

    void removeIfHas(size_t index) noexcept override {
        if (has(index)) {
            remove(index);
        }
    }

    std::span<const size_t> indices() const noexcept {
        return std::span<const size_t>(componentToIndex_);
    }

private:
    static constexpr int32_t EMPTY = -1;

    std::vector<int32_t> indexToComponent_;
    std::vector<T> components_;
    std::vector<size_t> componentToIndex_;
};


template<typename... Storages>
class ComponentFilter {
public:
    ComponentFilter(const Storages*... storages) : storages_(storages...) {
        assert(true);
    }

    ComponentFilter(const ComponentFilter&) = default;
    ComponentFilter& operator=(const ComponentFilter&) = default;
    ComponentFilter(ComponentFilter&&) = default;
    ComponentFilter& operator=(ComponentFilter&&) = default;

    bool check(size_t index) const noexcept {
        return (std::get<const Storages*>(storages_)->has(index) && ...);
    }

private:
    std::tuple<const Storages*...> storages_;
};


class ComponentManager {
public:
    ComponentManager() = default;
    ~ComponentManager() = default;

    ComponentManager(const ComponentManager&) = delete;
    ComponentManager& operator=(const ComponentManager&) = delete;
    ComponentManager(ComponentManager&&) noexcept = default;
    ComponentManager& operator=(ComponentManager&&) noexcept = default;

    template <typename Component>
    bool has(size_t index) const noexcept {
        auto it = storages_.find(key<Component>());
        return it != storages_.end()
            && as<Component>(*it->second).has(index);
    }

    template <typename Component>
    size_t count() const noexcept {
        auto it = storages_.find(key<Component>());
        return it != storages_.end()
            ? as<Component>(*it->second).size()
            : 0;
    }

    template <typename Component>
    Component& get(size_t index) noexcept {
        assert(has<Component>(index));
        return getStorage<Component>().get(index);
    }

    template <typename Component>
    void insert(size_t index, Component&& value) noexcept {
        ensureStorage<Component>().insert(
            index, std::forward<Component>(value));
    }

    template <typename Component>
    void remove(size_t index) noexcept {
        assert(has<Component>(index));
        getStorage<Component>().remove(index);
    }

    template <typename Component>
    void removeAll() noexcept {
        ensureStorage<Component>().clear();
    }

    void clearIndex(size_t index) noexcept {
        for (auto& it : storages_) {
            it.second->removeIfHas(index);
        }
    }

    template<typename Master, typename... Pos>
    auto view() noexcept {
        return ensureStorage<Master>().indices()
            | std::ranges::views::filter(
                [filter=ComponentFilter(&ensureStorage<Pos>()...)](
                    size_t index
                ) {
                    return filter.check(index);
                });
    }

private:
    std::unordered_map<
        std::type_index,
        std::unique_ptr<AbstractComponentStorage>
    > storages_;

    template <typename Component>
    static std::type_index key() {
        static std::type_index index(typeid(Component));
        return index;
    }

    template <typename Component>
    static ComponentStorage<Component>& as(AbstractComponentStorage& storage) {
        assert(dynamic_cast<ComponentStorage<Component>*>(&storage));
        return reinterpret_cast<ComponentStorage<Component>&>(storage);
    }

    template <typename Component>
    ComponentStorage<Component>& getStorage() {
        auto it = storages_.find(key<Component>());
        assert(it != storages_.end());
        return as<Component>(*it->second);
    }

    template <typename Component>
    ComponentStorage<Component>& ensureStorage() {
        auto k = key<Component>();
        auto it = storages_.find(k);
        if (it == storages_.end()) {
            auto result = storages_.emplace(
                k,
                std::make_unique<ComponentStorage<Component>>());
            assert(result.second == true);
            it = result.first;
        }
        return as<Component>(*it->second);
    }
};

}  // namespace Istok::ECS::Internal

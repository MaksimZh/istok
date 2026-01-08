// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <cassert>
#include <cstddef>
#include <vector>

namespace Istok::ECS {

template <typename T>
class ComponentStorageOf {
public:
    ComponentStorageOf() = default;
    ~ComponentStorageOf() = default;
    
    ComponentStorageOf(const ComponentStorageOf&) = delete;
    ComponentStorageOf& operator=(const ComponentStorageOf&) = delete;
    ComponentStorageOf(ComponentStorageOf&&) noexcept = default;
    ComponentStorageOf& operator=(ComponentStorageOf&&) noexcept = default;

    size_t size() const {
        return components_.size();
    }

    bool has(size_t index) const {
        return index < indexToComponent_.size()
            && indexToComponent_[index] >= 0;
    }

    T& get(size_t index) {
        assert(has(index));
        return components_[indexToComponent_[index]];
    }

    void insert(size_t index, T&& value) {
        if (has(index)) {
            components_[indexToComponent_[index]] = std::move(value);
            return;
        }
        if (index >= indexToComponent_.size()) {
            indexToComponent_.resize(index + 1, EMPTY);
        }
        indexToComponent_[index] = components_.size();
        components_.push_back(std::move(value));
        componentToIndex_.push_back(index);
    }

    void remove(size_t index) {
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

private:
    static constexpr int32_t EMPTY = -1;

    std::vector<int32_t> indexToComponent_;
    std::vector<T> components_;
    std::vector<size_t> componentToIndex_;
};

}  // namespace Istok::ECS

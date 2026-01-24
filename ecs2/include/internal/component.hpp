// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once
#include <cassert>
#include <cstddef>
#include <vector>
#include <span>
#include <unordered_map>
#include <typeindex>
#include <memory>

namespace Istok::ECS::Internal {

class AbstractComponentStorage {
public:
    virtual ~AbstractComponentStorage() = default;
};

template <typename T>
class ComponentStorage : public AbstractComponentStorage {
public:
    ComponentStorage() = default;
    ~ComponentStorage() = default;
    
    ComponentStorage(const ComponentStorage&) = delete;
    ComponentStorage& operator=(const ComponentStorage&) = delete;
    ComponentStorage(ComponentStorage&&) noexcept = default;
    ComponentStorage& operator=(ComponentStorage&&) noexcept = default;

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

    std::span<const size_t> indices() const {
        return std::span<const size_t>(componentToIndex_);
    }

private:
    static constexpr int32_t EMPTY = -1;

    std::vector<int32_t> indexToComponent_;
    std::vector<T> components_;
    std::vector<size_t> componentToIndex_;
};


template<typename... Storages>
class PosComponentFilter {
public:
    PosComponentFilter(const Storages*... storages) : storages_(storages...) {
        assert(true);
    }

    PosComponentFilter(const PosComponentFilter&) = default;
    PosComponentFilter& operator=(const PosComponentFilter&) = default;
    PosComponentFilter(PosComponentFilter&&) = default;
    PosComponentFilter& operator=(PosComponentFilter&&) = default;

    bool check(size_t index) const {
        return (std::get<const Storages*>(storages_)->has(index) && ...);
    }

private:
    std::tuple<const Storages*...> storages_;
};

template<typename... Components>
class NegComponentFilter {};


template<typename Master, typename PosFilter, typename NegFilter>
class ComponentView {
public:
    class Iterator {
    public:
        using element_type = size_t;
        using difference_type = ptrdiff_t;
        
        Iterator(
            std::span<const size_t>::iterator current,
            std::span<const size_t>::iterator sentinel,
            const PosFilter& pos
        ) : current_(current), sentinel_(sentinel), pos_(&pos) {
            assert(current_ <= sentinel_);
            seek();
        }
        
        size_t operator*() const noexcept {
            assert(current_ < sentinel_);
            return *current_;
        }

        Iterator& operator++() noexcept {
            ++current_;
            seek();
            return *this;
        }

        Iterator operator++(int) noexcept {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const Iterator& other) const noexcept {
            assert(sentinel_ == other.sentinel_);
            assert(pos_ == other.pos_);
            return current_ == other.current_;
        }
    
    private:
        std::span<const size_t>::iterator current_;
        std::span<const size_t>::iterator sentinel_;
        const PosFilter* pos_;

        void seek() {
            while (current_ < sentinel_ && !pos_->check(*current_)) {
                current_++;
            }
        }
    };
    
    using iterator = Iterator;
    
    iterator begin() noexcept {
        return Iterator(
            master_.indices().begin(),
            master_.indices().end(),
            pos_);
    }
    
    iterator end() noexcept {
        return Iterator(
            master_.indices().end(),
            master_.indices().end(),
            pos_);
    }

    using const_iterator = Iterator;
    
    const_iterator begin() const noexcept {
        return Iterator(
            master_.indices().begin(),
            master_.indices().end(),
            pos_);
    }
    
    const_iterator end() const noexcept {
        return Iterator(
            master_.indices().end(),
            master_.indices().end(),
            pos_);
    }

    ComponentView(
        const Master& master,
        const PosFilter& pos,
        const NegFilter& neg
    ) : master_(master), pos_(pos) {}

    ComponentView(const ComponentView&) = delete;
    ComponentView& operator=(const ComponentView&) = delete;
    ComponentView(ComponentView&&) = delete;
    ComponentView& operator=(ComponentView&&) = delete;

private:
    const Master& master_;
    PosFilter pos_;
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
    bool has(size_t index) {
        auto it = storages_.find(key<Component>());
        return it != storages_.end()
            && as<Component>(*it->second).has(index);
    }

    template <typename Component>
    Component& get(size_t index) {
        assert(has<Component>(index));
        return getStorage<Component>().get(index);
    }

    template <typename Component>
    void insert(size_t index, Component&& value) {
        ensureStorage<Component>().insert(index, std::move(value));
    }

    template <typename Component>
    void remove(size_t index) {
        assert(has<Component>(index));
        getStorage<Component>().remove(index);
    }

    template<typename Master, typename... Pos>
    auto view() {
        return ComponentView(
            ensureStorage<Master>(),
            PosComponentFilter(&ensureStorage<Pos>()...),
            NegComponentFilter<>()
        );
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

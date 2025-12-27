// datastruct.hpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <tools/queue.hpp>

#include <cassert>
#include <vector>
#include <unordered_map>
#include <ranges>

namespace Istok::ECS {

template <typename T>
class DenseIterator {
public:
    using element_type = T;
    using difference_type = ptrdiff_t;

    DenseIterator() = default;
    DenseIterator(T* start) : current(start) {}

    T& operator*() const { return *current; }

    DenseIterator& operator++() {
        ++current;
        return *this;
    }

    DenseIterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const DenseIterator& other) const = default;

    operator DenseIterator<const T>() const {
        return DenseIterator<const T>(current);
    }

private:
    T* current;
};


template <typename T>
class DenseSafeScanner {
public:
    DenseSafeScanner() = default;
    DenseSafeScanner(DenseIterator<const T> current, DenseIterator<const T> sentinel)
        : current(current), sentinel(sentinel) {}

    bool finished() const {
        return current == sentinel;
    }

    const DenseIterator<const T>& get() const {
        return current;
    }
    
    void inc() {
        if (!finished()) {
            ++current;
        }
    }
    
    bool operator==(const DenseSafeScanner& other) const = default;

private:
    DenseIterator<const T> current;
    DenseIterator<const T> sentinel;
};


template <typename T>
class DenseRange {
public:
    using iterator = DenseIterator<T>;
    using const_iterator = DenseIterator<const T>;
    
    DenseRange(DenseIterator<T> start, DenseIterator<T> sentinel)
        : start(start), sentinel(sentinel) {}
    
    iterator begin() noexcept { return start; }
    iterator end() noexcept { return sentinel; }
    const_iterator begin() const noexcept { return start; }
    const_iterator end() const noexcept { return sentinel; }
    const_iterator cbegin() const noexcept { return start; }
    const_iterator cend() const noexcept { return sentinel; }

private:
    DenseIterator<T> start;
    DenseIterator<T> sentinel;
};


template <typename T>
class DenseArray {
public:
    DenseArray() = default;
    DenseArray(const DenseArray& other) = delete;
    DenseArray& operator=(const DenseArray& other) = delete;
    DenseArray(DenseArray&& other) noexcept = default;
    DenseArray& operator=(DenseArray&& other) noexcept = default;

    size_t size() const noexcept {
        return container.size();
    }
    
    T& operator[](size_t index) {
        assert(index < size());
        return container[index];
    }
    
    const T& operator[](size_t index) const {
        assert(index < size());
        return container[index];
    }

    void pushBack(const T& value) {
        container.push_back(value);
    }

    void pushBack(T&& value) {
        container.push_back(std::move(value));
    }

    void erase(size_t index) {
        assert(index < size());
        if (index < size() - 1) {
            container[index] = std::move(container.back());
        }
        container.pop_back();
    }

    void clear() {
        container.clear();
    }

    DenseRange<T> byElement() noexcept {
        T* start = container.data();
        return DenseRange<T>(start, start + container.size());
    }

    DenseRange<const T> byElement() const noexcept {
        const T* start = container.data();
        return DenseRange<T>(start, start + container.size());
    }

private:
    std::vector<T> container;
};


template <typename T1, typename T2>
class DenseArrayPair {
public:
    DenseArrayPair() = default;
    DenseArrayPair(const DenseArrayPair & other) = delete;
    DenseArrayPair & operator=(const DenseArrayPair & other) = delete;
    DenseArrayPair(DenseArrayPair && other) noexcept = default;
    DenseArrayPair & operator=(DenseArrayPair && other) noexcept = default;

    size_t size() const noexcept {
        assert(container1.size() == container2.size());
        return container1.size();
    }

    T1 &first(size_t index) {
        assert(index < size());
        return container1[index];
    }

    const T1 &first(size_t index) const {
        assert(index < size());
        return container1[index];
    }

    T2 &second(size_t index) {
        assert(index < size());
        return container2[index];
    }

    const T2 &second(size_t index) const {
        assert(index < size());
        return container2[index];
    }

    template <typename V1, typename V2>
    void pushBack(V1&& value1, V2&& value2)
        requires (
            std::same_as<std::decay_t<V1>, T1> &&
            std::same_as<std::decay_t<V2>, T2>)
    {
        container1.pushBack(std::forward<V1>(value1));
        container2.pushBack(std::forward<V2>(value2));
    }

    template <typename V1, typename V2>
    void set(size_t index, V1&& value1, V2&& value2)
        requires (
            std::same_as<std::decay_t<V1>, T1> &&
            std::same_as<std::decay_t<V2>, T2>)
    {
        assert(index < size());
        container1[index] = std::forward<V1>(value1);
        container2[index] = std::forward<V2>(value2);
    }

    void erase(size_t index) {
        assert(index < size());
        container1.erase(index);
        container2.erase(index);
    }

    void clear() {
        container1.clear();
        container2.clear();
    }

    DenseRange< T1> firstElements() noexcept {
        return container1.byElement();
    }

    DenseRange<const T1> firstElements() const noexcept {
        return container1.byElement();
    }

    DenseRange< T2> secondElements() noexcept {
        return container2.byElement();
    }

    DenseRange<const T2> secondElements() const noexcept {
        return container2.byElement();
    }

private:
    DenseArray<T1> container1;
    DenseArray<T2> container2;
};


template <typename T, typename Hash = std::hash<T>>
class IndexMap {
public:
    IndexMap() = default;
    IndexMap(const IndexMap &) = delete;
    IndexMap & operator=(const IndexMap &) = delete;
    IndexMap(IndexMap &&) noexcept = default;
    IndexMap & operator=(IndexMap &&) noexcept = default;

    bool contains(const T& key) const {
        return container.contains(key);
    }

    void insert(const T& key, size_t index) {
        container[key] = index;
    }
    
    void insert(T&& key, size_t index) {
        container[key] = index;
    }

    size_t get(const T& key) const {
        assert(contains(key));
        return container.find(key)->second;
    }

    void erase(const T& key) {
        assert(contains(key));
        container.erase(key);
    }

    void clear() {
        container.clear();
    }

private:
    std::unordered_map<T, size_t, Hash> container;
};


template <typename ID, typename V, typename Hash = std::hash<ID>>
class DenseMap {
public:
    DenseMap() = default;
    DenseMap(const DenseMap&) = delete;
    DenseMap& operator=(const DenseMap&) = delete;
    DenseMap(DenseMap&&) noexcept = default;
    DenseMap& operator=(DenseMap&&) noexcept = default;

    size_t size() const {
        return values.size();
    }

    bool contains(const ID& key) const {
        return indices.contains(key);
    }

    template <typename KVal, typename VVal>
    void insert(KVal&& key, VVal&& value)
        requires (
            std::same_as<std::decay_t<KVal>, ID> &&
            std::same_as<std::decay_t<VVal>, V>)
    {
        if (contains(key)) {
            size_t index = indices.get(key);
            values.set(index, std::forward<KVal>(key), std::forward<VVal>(value));
            return;
        }
        size_t index = values.size();
        indices.insert(std::forward<KVal>(key), index);
        values.pushBack(std::forward<KVal>(key), std::forward<VVal>(value));
    }

    V& get(const ID& key) {
        assert(contains(key));
        size_t index = indices.get(key);
        return values.second(index);
    }

    const V& get(const ID& key) const {
        assert(contains(key));
        size_t index = indices.get(key);
        return values.second(index);
    }

    void erase(const ID& key) {
        assert(contains(key));
        size_t index = indices.get(key);
        indices.erase(key);
        values.erase(index);
        if (index < size()) {
            indices.insert(values.first(index), index);
        }
    }

    void clear() {
        indices.clear();
        values.clear();
    }

    DenseRange< ID> byKey() noexcept {
        return values.firstElements();
    }

    DenseRange<const ID> byKey() const noexcept {
        return values.firstElements();
    }

private:
    IndexMap<ID, Hash> indices;
    DenseArrayPair<ID, V> values;
};


class LimitedCounter {
public:
    explicit LimitedCounter(size_t sentinel)
    : value_(0), sentinel_(sentinel) {}

    size_t sentinel() const {
        return sentinel_;
    }

    bool full() const {
        return value_ >= sentinel_;
    }

    size_t take() {
        assert(!full());
        return value_++;
    }

    void extend(size_t delta) {
        sentinel_ += delta;
    }

private:
    size_t value_;
    size_t sentinel_;
};


class IndexPool {
public:
    IndexPool(size_t initialSize) : nextIndex_(initialSize) {}

    size_t capacity() const {
        return nextIndex_.sentinel();
    }
    
    bool full() const {
        return nextIndex_.full() && freeIndices_.empty();
    }

    size_t getFreeIndex() {
        assert(!full());
        return freeIndices_.empty() ? nextIndex_.take() : freeIndices_.take();
    }
    
    void freeIndex(size_t index) {
        freeIndices_.push(index);
    }

    void extend(size_t delta) {
        nextIndex_.extend(delta);
    }

private:
    LimitedCounter nextIndex_;
    Istok::Tools::SimpleQueue<size_t> freeIndices_;
};


class CounterArray {
public:
    CounterArray(size_t initialSize) : values(initialSize) {}

    size_t size() const {
        return values.size();
    }

    size_t get(size_t index) const {
        assert(index < size());
        return values[index];
    }

    void inc(size_t index) {
        assert(index < size());
        ++values[index];
    }

    void extend(size_t delta) {
        values.resize(values.size() + delta);
    }

private:
    std::vector<size_t> values;
};


template <typename C>
class ContainerFilter {
public:
    ContainerFilter() = default;

    template <std::ranges::forward_range R>
    explicit ContainerFilter(R&& positive)
            : positive(
                std::ranges::begin(positive),
                std::ranges::end(positive)) {}

    template <std::ranges::forward_range R, std::ranges::forward_range RN>
    explicit ContainerFilter(R&& positive, RN&& negative)
            : positive(
                std::ranges::begin(positive),
                std::ranges::end(positive)),
            negative(
                std::ranges::begin(negative),
                std::ranges::end(negative)) {}
    
    template <typename T>
    bool operator()(T x) const {
        for (const C& c : positive) {
            if (!c.has(x)) {
                return false;
            }
        }
        for (const C& c : negative) {
            if (c.has(x)) {
                return false;
            }
        }
        return true;
    }

    template <std::ranges::forward_range RN>
    ContainerFilter exclude(RN&& neg) const {
        std::vector<std::reference_wrapper<C>> newNegative(negative);
        for (auto& n : neg) {
            newNegative.push_back(n);
        }
        return ContainerFilter(positive, newNegative);
    }


private:
    std::vector<std::reference_wrapper<C>> positive;
    std::vector<std::reference_wrapper<C>> negative;
};


} // namespace Istok::ECS

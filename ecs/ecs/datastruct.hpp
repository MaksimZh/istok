// datastruct.hpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <queue>
#include <vector>
#include <unordered_map>
#include <ranges>

namespace Istok::ECS {

template <typename T>
class Queue {
public:
    Queue() = default;
    Queue(const Queue& other) = delete;
    Queue& operator=(const Queue& other) = delete;
    Queue(Queue&& other) noexcept = default;
    Queue& operator=(Queue&& other) noexcept = default;
    
    bool empty() const {
        return container.empty();
    }

    void push(const T& value) {
        container.push(value);
    }

    void push(T&& value) {
        container.push(std::move(value));
    }

    T& front() {
        return container.front();
    }

    void pop() {
        container.pop();
    }

private:
    std::queue<T> container;
};


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

private:
    std::unordered_map<T, size_t, Hash> container;
};


template <typename K, typename V, typename Hash = std::hash<K>>
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

    bool contains(const K& key) const {
        return indices.contains(key);
    }

    template <typename KVal, typename VVal>
    void insert(KVal&& key, VVal&& value)
        requires (
            std::same_as<std::decay_t<KVal>, K> &&
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

    V& get(const K& key) {
        assert(contains(key));
        size_t index = indices.get(key);
        return values.second(index);
    }

    const V& get(const K& key) const {
        assert(contains(key));
        size_t index = indices.get(key);
        return values.second(index);
    }

    void erase(const K& key) {
        assert(contains(key));
        size_t index = indices.get(key);
        indices.erase(key);
        values.erase(index);
        if (index < size()) {
            indices.insert(values.first(index), index);
        }
    }

    DenseRange< K> byKey() noexcept {
        return values.firstElements();
    }

    DenseRange<const K> byKey() const noexcept {
        return values.firstElements();
    }

private:
    IndexMap<K, Hash> indices;
    DenseArrayPair<K, V> values;
};


class LimitedCounter {
public:
    explicit LimitedCounter(size_t sentinel) : value(0), sentinel(sentinel) {}

    bool full() const {
        return value >= sentinel;
    }

    size_t get() const {
        return value;
    }

    void inc() {
        assert(!full());
        ++value;
    }

    void extend(size_t delta) {
        sentinel += delta;
    }

private:
    size_t value;
    size_t sentinel;
};


class IndexPool {
public:
    IndexPool(size_t initialSize) : nextIndex(initialSize) {}

    size_t getFreeIndex() {
        assert(!full());
        if (freeIndices.empty()) {
            size_t index = nextIndex.get();
            nextIndex.inc();
            return index;
        }
        size_t index = freeIndices.front();
        freeIndices.pop();
        return index;
    }
    
    void freeIndex(size_t index) {
        freeIndices.push(index);
    }
    
    bool full() const {
        return nextIndex.full() && freeIndices.empty();
    }

    void extend(size_t delta) {
        nextIndex.extend(delta);
    }

private:
    LimitedCounter nextIndex;
    Queue<size_t> freeIndices;
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

} // namespace Istok::ECS

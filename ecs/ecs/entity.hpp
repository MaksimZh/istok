// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <cassert>
#include <queue>
#include <vector>


struct EntityIndex {
    uint64_t value;

    constexpr EntityIndex() : value(0) {}
    explicit constexpr EntityIndex(uint64_t value) : value(value) {}
    bool operator==(const EntityIndex& other) const = default;

    EntityIndex& operator++() {
        ++value;
        return *this;
    }

    EntityIndex operator++(int) {
        auto tmp = *this;
        ++*this;
        return *this;
    }

    operator size_t() const {
        return static_cast<size_t>(value);
    }
};


struct EntityGeneration {
    uint64_t value;

    constexpr EntityGeneration() : value(0) {}
    explicit constexpr EntityGeneration(uint64_t v) : value(v) {}
    bool operator==(const EntityGeneration& other) const = default;

    EntityGeneration& operator++() {
        ++value;
        return *this;
    }

    EntityGeneration operator++(int) {
        auto tmp = *this;
        ++*this;
        return *this;
    }
};


struct Entity {
    static constexpr uint64_t lowerMask = 0x00000000ffffffff;
    static constexpr uint64_t upperMask = 0xffffffff00000000;
    
    uint64_t value;

    constexpr Entity() : value(0) {}
    constexpr Entity(EntityIndex index, EntityGeneration generation)
        : value(index.value + (generation.value << 32)) {}
    bool operator==(const Entity& other) const = default;
    
    EntityIndex index() const {
        return EntityIndex(value & lowerMask);
    }

    EntityGeneration generation() const {
        return EntityGeneration((value & upperMask) >> 32);
    }
};


class LimitedCounter {
public:
    LimitedCounter(size_t value, size_t sentinel)
        : value(value), sentinel(sentinel) {}
    
    bool isFull() const {
        return value == sentinel;
    }

    void extendBy(size_t size) {
        sentinel += size;
    }

    operator size_t() const {
        return value;
    }

    LimitedCounter& operator++() {
        assert(!isFull());
        ++value;
        return *this;
    }

    LimitedCounter operator++(int) {
        assert(!isFull());
        auto tmp = *this;
        ++*this;
        return tmp;
    }

private:
    size_t value;
    size_t sentinel;
};


template <typename T>
class Queue {
public:
    bool empty() const {
        return container.empty();
    }

    void push(T value) {
        container.push(value);
    }

    T pop() {
        T value = container.front();
        container.pop();
        return value;
    }

private:
    std::queue<T> container;
};


class EntityIndexPool {
public:
    EntityIndexPool(size_t initialSize) : nextIndex(0, initialSize) {}

    EntityIndex getFreeIndex() {
        assert(!isFull());
        if (!freeIndices.empty()) {
            return freeIndices.pop();
        }
        return EntityIndex(nextIndex++);
    }
    
    void freeIndex(EntityIndex index) {
        freeIndices.push(index);
    }
    
    bool isFull() const {
        return nextIndex.isFull() && freeIndices.empty();
    }

    void extendBy(size_t size) {
        nextIndex.extendBy(size);
    }

private:
    LimitedCounter nextIndex;
    Queue<EntityIndex> freeIndices;
};


class GenerationArray {
public:
    GenerationArray(size_t initialSize) : values(initialSize) {}

    size_t size() const {
        return values.size();
    }

    EntityGeneration& operator[](size_t index) {
        assert(index < values.size());
        return values[index];
    }

    const EntityGeneration& operator[](size_t index) const {
        assert(index < values.size());
        return values[index];
    }

    void extendBy(size_t size) {
        values.resize(values.size() + size);
    }

private:
    std::vector<EntityGeneration> values;
};


class EntityStorage {
public:
    EntityStorage(size_t initialSize)
        : indexPool(initialSize), generations(initialSize) {}
    
    Entity create() {
        EntityIndex index = indexPool.getFreeIndex();
        return Entity(index, generations[index]);
    }

    size_t size() const {
        return generations.size();
    }

    void destroy(Entity e) {
        indexPool.freeIndex(e.index());
        generations[e.index()]++;
    }
    
    bool isValid(Entity e) const {
        return generations[e.index()] == e.generation();
    }

    bool isFull() const {
        return indexPool.isFull();
    }
    
    void extendBy(size_t size) {
        indexPool.extendBy(size);
        generations.extendBy(size);
    }

private:
    EntityIndexPool indexPool;
    GenerationArray generations;
};


class EntityManager {
public:
    EntityManager(size_t initialCapacity)
        : storage(initialCapacity) {}
    
    Entity create() {
        if (storage.isFull()) {
            storage.extendBy(storage.size());
        }
        return storage.create();
    }

    void destroy(Entity e) {
        storage.destroy(e);
    }
    
    bool isValid(Entity e) const {
        return storage.isValid(e);
    }

private:
    EntityStorage storage;
};

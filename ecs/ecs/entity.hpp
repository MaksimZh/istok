// entity.hpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "datastruct.hpp"

#include <cassert>


namespace Istok::ECS {

struct Entity {
    uint64_t value;

    constexpr Entity() : value(0) {}
    constexpr Entity(size_t index, size_t generation)
        : value(index + (generation << 32)) {}
    bool operator==(const Entity& other) const = default;

    uint32_t index() const {
        return value & LOWER_MASK;
    }

    uint32_t generation() const {
        return (value & UPPER_MASK) >> 32;
    }

    struct Hasher {
        size_t operator()(const Entity& entity) const {
            return std::hash<uint64_t>()(entity.value);
        }
    };

private:
    static constexpr uint64_t LOWER_MASK = 0x00000000ffffffff;
    static constexpr uint64_t UPPER_MASK = 0xffffffff00000000;
};


class EntityStorage {
public:
    EntityStorage(size_t initialSize)
        : indexPool(initialSize), generations(initialSize) {}

    size_t size() const {
        return generations.size();
    }

    bool full() const {
        return indexPool.full();
    }
    
    Entity create() {
        assert(!full());
        size_t index = indexPool.getFreeIndex();
        generations.inc(index);
        return Entity(index, generations.get(index));
    }

    void destroy(Entity e) {
        indexPool.freeIndex(e.index());
        generations.inc(e.index());
    }
    
    bool isValid(Entity e) const {
        return e.generation() % 2 == 1
            && generations.get(e.index()) == e.generation();
    }
   
    void extend(size_t delta) {
        indexPool.extend(delta);
        generations.extend(delta);
    }

private:
    IndexPool indexPool;
    CounterArray generations;
};


class EntityManager {
public:
    EntityManager(size_t initialCapacity)
        : storage(initialCapacity) {}
    
    Entity create() {
        if (storage.full()) {
            storage.extend(storage.size());
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

} // namespace Istok::ECS

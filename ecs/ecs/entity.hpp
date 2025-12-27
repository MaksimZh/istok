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
    EntityStorage(size_t initialCapacity)
        : indexPool_(initialCapacity), generations_(initialCapacity) {}

    size_t capacity() const {
        return generations_.size();
    }

    bool isFull() const {
        return indexPool_.full();  // TODO: isFull
    }
    
    Entity createEntity() {
        assert(!isFull());
        size_t index = indexPool_.getFreeIndex();
        generations_.inc(index);
        return Entity(index, generations_.get(index));
    }

    void destroyEntity(Entity e) {
        indexPool_.freeIndex(e.index());
        generations_.inc(e.index());
    }
    
    bool isValidEntity(Entity e) const {
        return e.index() < indexPool_.capacity()
            && e.generation() % 2 == 1
            && generations_.get(e.index()) == e.generation();
    }
   
    void extend(size_t delta) {
        indexPool_.extend(delta);
        generations_.extend(delta);
    }

private:
    IndexPool indexPool_;
    CounterArray generations_;
};


class EntityManager {
public:
    EntityManager(size_t initialCapacity)
        : storage(initialCapacity) {}
    
    Entity create() {  // TODO: createEntity
        if (storage.isFull()) {
            storage.extend(storage.capacity());
        }
        return storage.createEntity();
    }

    void destroy(Entity e) {
        storage.destroyEntity(e);
    }
    
    bool isValid(Entity e) const {
        return storage.isValidEntity(e);
    }

private:
    EntityStorage storage;
};

} // namespace Istok::ECS

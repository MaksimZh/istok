// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <bit>
#include <cassert>
#include <cstdint>
#include <vector>


namespace Istok::ECS {

struct Entity {
    struct Hasher {
        size_t operator()(const Entity& entity) const {
            return std::bit_cast<size_t>(entity);
        }
    };

    constexpr Entity(size_t index, size_t generation)
    : index_(index), generation_(generation) {}
    
    bool operator==(const Entity& other) const = default;

    uint32_t index() const {
        return index_;
    }

    uint32_t generation() const {
        return generation_;
    }

private:
    int32_t index_;
    int32_t generation_;
};


class EntityManager {
public:
    bool isValidEntity(Entity entity) const {
        return entity.index() < cells_.size()
            && std::bit_cast<Entity>(cells_[entity.index()]) == entity;
    }
    
    Entity createEntity() {
        if (freeIndex_ == cells_.size()) {
            cells_.push_back(Cell{freeIndex_, 0});
            return getEntity(freeIndex_++);
        }
        auto index = freeIndex_;
        freeIndex_ = getLink(freeIndex_);
        reviveEntity(index);
        return getEntity(index);
    }

    void deleteEntity(Entity entity) {
        if (!isValidEntity(entity)) {
            return;
        }
        setLink(entity.index(), freeIndex_);
        freeIndex_ = entity.index();
    }

private:
    struct Cell {
        int32_t index;
        int32_t generation;
    };

    std::vector<Cell> cells_;
    int32_t freeIndex_ = 0;

    bool isLink(int32_t index) const {
        assert(index >= 0);
        assert(index < cells_.size());
        return cells_[index].index < 0;
    }

    Entity getEntity(int32_t index) const {
        assert(index >= 0);
        assert(index < cells_.size());
        assert(!isLink(index));
        return Entity(cells_[index].index, cells_[index].generation);
    }

    int32_t getLink(int32_t index) const {
        assert(index >= 0);
        assert(index < cells_.size());
        assert(isLink(index));
        return -cells_[index].index - 1;
    }

    void setLink(int32_t index, int32_t target) {
        assert(index >= 0);
        assert(index < cells_.size());
        assert(target >= 0);
        assert(target <= cells_.size());
        cells_[index].index = -target - 1;
    }

    void reviveEntity(int32_t index) {
        assert(index >= 0);
        assert(index < cells_.size());
        assert(isLink(index));
        cells_[index].index = index;
        ++cells_[index].generation;
    }
};

}  // namespace Istok::ECS

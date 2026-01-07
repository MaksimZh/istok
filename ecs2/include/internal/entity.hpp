// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <bit>
#include <cassert>
#include <cstdint>
#include <vector>


namespace Istok::ECS {

struct Entity final {
    struct Hasher {
        size_t operator()(const Entity& entity) const {
            return std::bit_cast<size_t>(entity);
        }
    };
       
    bool operator==(const Entity& other) const = default;

    uint32_t index() const {
        return index_;
    }

private:
    friend class EntityManager;
    
    int32_t index_;
    int32_t generation_;

    constexpr Entity(int32_t index, int32_t generation)
    : index_(index), generation_(generation) {}
};


class EntityManager final {
public:
    EntityManager() = default;
    ~EntityManager() = default;
    
    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(const EntityManager&) = delete;
    EntityManager(EntityManager&&) noexcept = default;
    EntityManager& operator=(EntityManager&&) noexcept = default;

    bool isValidEntity(Entity entity) const {
        return entity.index_ < entities_.size()
            && std::bit_cast<Entity>(entities_[entity.index_]) == entity;
    }
    
    Entity createEntity() {
        if (freeIndex_ == entities_.size()) {
            entities_.push_back(Entity(freeIndex_, 0));
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
        setLink(entity.index_, freeIndex_);
        freeIndex_ = entity.index_;
    }

private:
    std::vector<Entity> entities_;
    int32_t freeIndex_ = 0;

    bool isLink(int32_t index) const {
        assert(index >= 0);
        assert(index < entities_.size());
        return entities_[index].index_ < 0;
    }

    Entity getEntity(int32_t index) const {
        assert(index >= 0);
        assert(index < entities_.size());
        assert(!isLink(index));
        return entities_[index];
    }

    int32_t getLink(int32_t index) const {
        assert(index >= 0);
        assert(index < entities_.size());
        assert(isLink(index));
        return -entities_[index].index_ - 1;
    }

    void setLink(int32_t index, int32_t target) {
        assert(index >= 0);
        assert(index < entities_.size());
        assert(target >= 0);
        assert(target <= entities_.size());
        entities_[index].index_ = -target - 1;
    }

    void reviveEntity(int32_t index) {
        assert(index >= 0);
        assert(index < entities_.size());
        assert(isLink(index));
        entities_[index].index_ = index;
        ++entities_[index].generation_;
    }
};

}  // namespace Istok::ECS

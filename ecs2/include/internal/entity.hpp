// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>


namespace Istok::ECS {

namespace Internal {
    class EntityManager;
}


struct Entity final {
    struct Hasher {
        size_t operator()(const Entity& entity) const {
            return std::bit_cast<size_t>(entity);
        }
    };
       
    bool operator==(const Entity& other) const = default;

    size_t index() const {
        return index_;
    }

private:
    friend class Internal::EntityManager;
    
    int32_t index_;
    int32_t generation_;

    constexpr Entity(int32_t index, int32_t generation)
    : index_(index), generation_(generation) {}
};


namespace Internal {

class EntityManager final {
public:
    EntityManager() = default;
    ~EntityManager() = default;
    
    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(const EntityManager&) = delete;
    EntityManager(EntityManager&&) noexcept = default;
    EntityManager& operator=(EntityManager&&) noexcept = default;

    bool isValidEntity(Entity entity) const noexcept {
        return entity.index_ < entities_.size()
            && std::bit_cast<Entity>(entities_[entity.index_]) == entity;
    }

    bool isValidIndex(size_t index) const noexcept {
        return index < entities_.size() && !isLink(index);
    }

    size_t size() const noexcept {
        return size_;
    }
    
    Entity createEntity() noexcept {
        ++size_;
        if (freeIndex_ == entities_.size()) {
            entities_.push_back(Entity(freeIndex_, 0));
            return getEntity(freeIndex_++);
        }
        auto index = freeIndex_;
        freeIndex_ = getLink(freeIndex_);
        reviveEntity(index);
        return getEntity(index);
    }

    Entity entityFromIndex(size_t index) const noexcept {
        assert(isValidIndex(index));
        return getEntity(index);
    }

    void deleteEntity(Entity entity) noexcept {
        assert(isValidEntity(entity));
        --size_;
        setLink(entity.index_, freeIndex_);
        freeIndex_ = entity.index_;
    }

private:
    std::vector<Entity> entities_;
    size_t freeIndex_ = 0;
    size_t size_ = 0;

    bool isLink(size_t index) const noexcept {
        assert(index >= 0);
        assert(index < entities_.size());
        return entities_[index].index_ < 0;
    }

    Entity getEntity(size_t index) const noexcept {
        assert(index >= 0);
        assert(index < entities_.size());
        assert(!isLink(index));
        return entities_[index];
    }

    size_t getLink(size_t index) const noexcept {
        assert(index >= 0);
        assert(index < entities_.size());
        assert(isLink(index));
        return -entities_[index].index_ - 1;
    }

    void setLink(size_t index, size_t target) noexcept {
        assert(index >= 0);
        assert(index < entities_.size());
        assert(target >= 0);
        assert(target <= entities_.size());
        entities_[index].index_ = -target - 1;
    }

    void reviveEntity(size_t index) noexcept {
        assert(index >= 0);
        assert(index < entities_.size());
        assert(isLink(index));
        entities_[index].index_ = index;
        ++entities_[index].generation_;
    }
};

}  // namespace Internal

}  // namespace Istok::ECS

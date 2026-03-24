// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <format>
#include <string>
#include <vector>


namespace Istok::ECS {

namespace Internal {
    class EntityManager;
    struct EntityEntry;
}


struct Entity final {
    struct Hasher {
        size_t operator()(const Entity& entity) const noexcept {
            return std::bit_cast<size_t>(entity);
        }
    };

    bool operator==(const Entity& other) const = default;

    size_t index() const noexcept {
        return index_;
    }

private:
    friend class Internal::EntityManager;
    friend struct Internal::EntityEntry;

    int32_t index_;
    int32_t generation_;

    constexpr Entity(int32_t index, int32_t generation)
    : index_(index), generation_(generation) {}
};


namespace Internal {

struct EntityEntry {
public:
    EntityEntry(size_t index) : index_(index), generation_(0) {}

    bool isLink() const noexcept {
        return index_ < 0;
    }

    Entity entity() const noexcept {
        assert(!isLink());
        return Entity{index_, generation_};
    }

    size_t link() const noexcept {
        assert(isLink());
        return -index_ - 1;
    }

    void setLink(size_t target) noexcept {
        index_ = -static_cast<int32_t>(target) - 1;
    }

    void setEntity(size_t index) noexcept {
        index_ = static_cast<int32_t>(index);
        ++generation_;
    }

    bool operator==(const Entity& other) const noexcept {
        return index_ == other.index_ && generation_ == other.generation_;
    }

private:
    int32_t index_;
    int32_t generation_;
};


class EntityManager final {
public:
    EntityManager() = default;
    ~EntityManager() = default;

    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(const EntityManager&) = delete;
    EntityManager(EntityManager&&) = default;
    EntityManager& operator=(EntityManager&&) = default;

    bool isValid(Entity entity) const noexcept {
        return entity.index_ < entities_.size()
            && entities_[entity.index_] == entity;
    }

    Entity get(size_t index) const noexcept {
        assert(index < entities_.size());
        assert(!entities_[index].isLink());
        return entities_[index].entity();
    }

    Entity create() noexcept {
        if (freeIndex_ == entities_.size()) {
            entities_.emplace_back(freeIndex_);
            return entities_[freeIndex_++].entity();
        }
        auto index = freeIndex_;
        freeIndex_ = entities_[index].link();
        entities_[index].setEntity(index);
        return entities_[index].entity();
    }

    void remove(Entity entity) noexcept {
        assert(isValid(entity));
        entities_[entity.index_].setLink(freeIndex_);
        freeIndex_ = entity.index_;
    }

private:
    std::vector<EntityEntry> entities_;
    size_t freeIndex_ = 0;
};

}  // namespace Internal

}  // namespace Istok::ECS


template <>
struct std::formatter<Istok::ECS::Entity> : std::formatter<std::string> {
    auto format(Istok::ECS::Entity entity, std::format_context& ctx) const {
        return std::formatter<std::string>::format(
            std::format("@{:d}", entity.index()),
            ctx);
    }
};

// ecs.hpp
// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "manager.hpp"
#include "system.hpp"

#include <logging.hpp>

namespace Istok::ECS {

class ECSManager final {
    CLASS_WITH_LOGGER_PREFIX("ECS", "ECS: ");
public:
    ECSManager() {}

    ECSManager(const ECSManager&) = delete;
    ECSManager& operator=(const ECSManager&) = delete;
    ECSManager(ECSManager&&) = delete;
    ECSManager& operator=(ECSManager&&) = delete;

    /* Calls clear method to ensure the reference to ECSManager is valid
     * until all systems and components are destroyed.
     */
    ~ECSManager() {
        clear();
    }

    void pushSystem(std::unique_ptr<System>&& system) {
        if (!system) {
            throw std::runtime_error("Null system pointer");
        }
        systems_.push(std::move(system));
    }

    void popSystem() {
        if (systems_.empty()) {
            throw std::runtime_error("No system to pop");
        }
        systems_.pop();
    }

    bool hasSystems() const {
        return systems_.empty();
    }

    /* The systems are destroyed first in reverse order (stack pop until empty).
     * The components are destroyed after the systems in undefined order.
     */
    void clear() {
        LOG_TRACE("clear");
        systems_.clear();
        ecm_.clear();
    }

    void iterate() {
        LOG_TRACE("iterate");
        systems_.run();
    }

    void run() {
        if (running_) {
            return;
        }
        LOG_TRACE("run");
        running_ = true;
        while (running_) {
            iterate();
        }
    }

    void stop() {
        LOG_TRACE("stop");
        running_ = false;
    }

    bool isValidEntity(Entity e) const {
        return ecm_.isValidEntity(e);
    }

    template <typename Component>
    bool has(Entity e) const {
        assert(isValidEntity(e));
        return ecm_.has<Component>(e);
    }

    template <typename Component>
    bool hasAny() const {
        return ecm_.hasAny<Component>();
    }

    template <typename Component>
    Component& get(Entity e) {
        assert(isValidEntity(e));
        assert(has<Component>(e));
        return ecm_.get<Component>(e);
    }

    template <typename Component>
    const Component& get(Entity e) const {
        assert(isValidEntity(e));
        assert(has<Component>(e));
        return ecm_.get<Component>(e);
    }

    template<typename... Components>
    Entity createEntity(Components&&... components) {
        Entity e = ecm_.createEntity();
        (ecm_.set(e, std::forward<Components>(components)), ...);
        return e;
    }

    void destroyEntity(Entity e) {
        assert(isValidEntity(e));
        ecm_.destroyEntity(e);
    }

    template <typename Component>
    void set(Entity e, Component&& component) {
        assert(isValidEntity(e));
        ecm_.set(e, std::move(component));
    }

    template <typename Component>
    void remove(Entity e) {
        assert(isValidEntity(e));
        assert(has<Component>(e));
        ecm_.remove<Component>(e);
    }

    template <typename Component>
    void removeAll() {
        ecm_.removeAll<Component>();
    }

    template<typename... Components>
    EntityView view() {
        return ecm_.view<Components...>();
    }

private:
    bool running_ = false;
    EntityComponentManager ecm_;
    SystemStack systems_;
};

}

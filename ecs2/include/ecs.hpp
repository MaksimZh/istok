// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include "internal/entity.hpp"
#include "internal/component.hpp"


namespace Istok::ECS {

class ECSManager {
public:
    bool isValidEntity(Entity entity) const {
        return entityManager.isValidEntity(entity);
    }
    
    Entity createEntity() {
        return entityManager.createEntity();
    }

    void deleteEntity(Entity entity) {
        entityManager.deleteEntity(entity);
    }

private:
    Internal::EntityManager entityManager;
    Internal::ComponentManager componentManager;
};

}  // namespace Istok::ECS

// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "winapi/base/environment2.hpp"

#include <functional>

#include <istok/ecs.hpp>
#include <istok/logging.hpp>

#include "winapi/base/dispatcher2.hpp"


namespace Istok::GUI::WinAPI {

bool runInEnvironment(
    ECS::ECSManager& ecs,
    std::move_only_function<bool(
        ECS::ECSManager&,
        ECS::Entity,
        Dispatcher2&
    )> func
) {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI", "WinAPI: ");
    using Dispatcher2Container = std::unique_ptr<Dispatcher2>;
    if (ecs.count<Dispatcher2Container>() != 1) {
        LOG_ERROR("Single Dispatcher2 expected.");
        return false;
    }
    ECS::Entity master = *ecs.view<Dispatcher2Container>().begin();
    Dispatcher2* dispatcher = ecs.get<Dispatcher2Container>(master).get();
    if (!dispatcher) {
        LOG_ERROR("Empty Dispatcher2 found on {}.", master);
        return false;
    }
    return func(ecs, master, *dispatcher);
}

bool runInEnvironment(
    ECS::ECSManager& ecs,
    std::move_only_function<bool(
        ECS::ECSManager&,
        Dispatcher2&
    )> func
) {
    return runInEnvironment(
        ecs,
        [func=std::move(func)](
            ECS::ECSManager& ecs, ECS::Entity master, Dispatcher2& dispatcher
        ) mutable {
            return func(ecs, dispatcher);
        });
}

}  // namespace Istok::GUI::WinAPI
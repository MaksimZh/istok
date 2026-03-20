// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "winapi/base/environment.hpp"

#include <functional>

#include <istok/ecs.hpp>
#include <istok/logging.hpp>

#include "winapi/base/dispatcher.hpp"
#include "winapi/base/winapi_delegate.hpp"

namespace Istok::GUI::WinAPI {

namespace {

bool runWithDispatcher(
    ECS::ECSManager& ecs, ECS::Entity master, WinAPIDelegate& winapi,
    std::move_only_function<bool(
        ECS::ECSManager&,
        ECS::Entity,
        WinAPIDelegate&,
        Dispatcher&
    )> func
) {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI", "WinAPI: ");
    using DispatcherContainer = std::unique_ptr<Dispatcher>;
    if (!ecs.has<DispatcherContainer>(master)) {
        LOG_ERROR("Dispatcher not found.");
        return false;
    }
    auto& dispatcher = ecs.get<DispatcherContainer>(master);
    if (!dispatcher) {
        LOG_ERROR("Empty Dispatcher found on {}.", master);
        return false;
    }
    return func(ecs, master, winapi, *dispatcher);
}

}  // namespace


bool runInEnvironment(
    ECS::ECSManager& ecs,
    std::move_only_function<bool(
        ECS::ECSManager&,
        ECS::Entity,
        WinAPIDelegate&
    )> func
) {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI", "WinAPI: ");
    using WinAPIContainer = std::unique_ptr<WinAPIDelegate>;
    if (ecs.count<WinAPIContainer>() != 1) {
        LOG_ERROR("Single WinAPIDelegate expected.");
        return false;
    }
    ECS::Entity master = *ecs.view<WinAPIContainer>().begin();
    WinAPIDelegate* winapi = ecs.get<WinAPIContainer>(master).get();
    if (!winapi) {
        LOG_ERROR("Empty WinAPIDelegate found on {}.", master);
        return false;
    }
    return func(ecs, master, *winapi);
}

bool runInEnvironment(
    ECS::ECSManager& ecs,
    std::move_only_function<bool(
        ECS::ECSManager&,
        WinAPIDelegate&
    )> func
) {
    return runInEnvironment(
        ecs,
        [func=std::move(func)](
            ECS::ECSManager& ecs, ECS::Entity master, WinAPIDelegate& winapi
        ) mutable {
            return func(ecs, winapi);
        });
}

bool runInEnvironment(
    ECS::ECSManager& ecs,
    std::move_only_function<bool(
        ECS::ECSManager&,
        ECS::Entity,
        WinAPIDelegate&,
        Dispatcher&
    )> func
) {
    return runInEnvironment(
        ecs,
        [func=std::move(func)](
            ECS::ECSManager& ecs, ECS::Entity master, WinAPIDelegate& winapi
        ) mutable {
            return runWithDispatcher(ecs, master, winapi, std::move(func));
        });
}

bool runInEnvironment(
    ECS::ECSManager& ecs,
    std::move_only_function<bool(
        ECS::ECSManager&,
        WinAPIDelegate&,
        Dispatcher&
    )> func
) {
    return runInEnvironment(
        ecs,
        [func=std::move(func)](
            ECS::ECSManager& ecs, ECS::Entity master,
            WinAPIDelegate& winapi, Dispatcher& dispatcher
        ) mutable {
            return func(ecs, winapi, dispatcher);
        });
}

}  // namespace Istok::GUI::WinAPI

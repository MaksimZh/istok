// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "messages.hpp"

#include <memory>

#include <istok/ecs.hpp>
#include <istok/logging.hpp>

#include "winapi/base/message_loop.hpp"
#include "winapi/base/delegate.hpp"
#include "winapi/base/dispatcher.hpp"

namespace Istok::GUI::WinAPI {

bool setupMessages(ECS::ECSManager& ecs) {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI", "WinAPI: ");
    using WinAPIContainer = std::unique_ptr<WinAPIDelegate>;
    if (ecs.count<WinAPIContainer>() != 1) {
        LOG_ERROR("Single WinAPIDelegate expected.");
        return false;
    }
    ECS::Entity master = *ecs.view<WinAPIContainer>().begin();
    LOG_DEBUG("Detected master entity {}", master);
    WinAPIDelegate* winapi = ecs.get<WinAPIContainer>(master).get();
    if (!winapi) {
        LOG_ERROR("Empty WinAPIDelegate found.");
        return false;
    }

    ecs.insert(master, std::make_unique<Dispatcher>(*winapi));
    ecs.addBottomLoopSystem(
        createMessageLoopSystem(*winapi, []() noexcept {}, master));
    return true;
}

}  // namespace Istok::GUI::WinAPI

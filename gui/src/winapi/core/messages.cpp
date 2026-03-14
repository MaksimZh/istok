// Copyright 2026 Maksim Sergeevich Zholudev. All rights reserved
#include "messages.hpp"

#include <memory>

#include <istok/ecs.hpp>
#include <istok/logging.hpp>

#include "winapi/base/delegate.hpp"
#include "winapi/base/dispatcher.hpp"


namespace Istok::GUI::WinAPI {

namespace {

struct ProcessingMessageFlag {};

void messageLoopIteration(
    WinAPIDelegate& winapi,
    ECS::Entity master,
    ECS::ECSManager& ecs
) noexcept {
    WITH_LOGGER_PREFIX("Istok.GUI.WinAPI.MessageLoop", "WinAPI: ");
    if (ecs.has<ProcessingMessageFlag>(master)) {
        LOG_TRACE("Skipping nested message loop iteration.");
        return;
    }
    ecs.insert(master, ProcessingMessageFlag{});
    MSG msg = winapi.getMessage();
    if (msg.message == WM_QUIT) {
        LOG_DEBUG("WM_QUIT message received.");
        ecs.insert(master, QuitFlag{});
        return;
    }
    winapi.dispatchMessage(msg);
    ecs.remove<ProcessingMessageFlag>(master);
}


}  // namespace

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

    ecs.addBottomLoopSystem([winapi, master](ECS::ECSManager& ecs) noexcept {
        messageLoopIteration(*winapi, master, ecs); });
    return true;
}

}  // namespace Istok::GUI::WinAPI

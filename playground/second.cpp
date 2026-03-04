#include <cassert>
#include <functional>
#include <memory>

#include <windows.h>

#include <istok/logging.hpp>
#include <istok/ecs.hpp>
#include <istok/winapi.hpp>

using namespace Istok;

struct QuitFlag { bool value; };
struct ProcessingMessageFlag { bool value; };

template <typename Component>
Component& getUnique(ECS::ECSManager& ecs) {
    assert(ecs.count<Component>() == 1);
    auto holder = *ecs.view<Component>().begin();
    return ecs.get<Component>(holder);
}

LRESULT wmCloseHandler(
    ECS::ECSManager& ecs, ECS::Entity entity, WinAPI::WindowMessage message
) noexcept {
    assert(message.msg == WM_CLOSE);
    if (!ecs.has<EventHandlers::Close>(entity)) {
        return WinAPI::handleMessageByDefault(message);
    }
    ecs.get<EventHandlers::Close>(entity).func();
    return 0;
}

LRESULT wmSizeHandler(
    ECS::ECSManager& ecs, ECS::Entity entity, WinAPI::WindowMessage message
) noexcept {
    assert(message.msg == WM_SIZE);
    ecs.iterate();
    return 0;
}

void setupMessageDispatcher(ECS::ECSManager& ecs, ECS::Entity master) {
    auto dispatcher = std::make_unique<WinAPI::WindowMessageDispatcher>(ecs);
    dispatcher->setHandler(WM_CLOSE, wmCloseHandler);
    dispatcher->setHandler(WM_SIZE, wmSizeHandler);
    ecs.insert(master, std::move(dispatcher));
}

struct Location {
    WinAPI::Rect<int> rect;
};

struct NewWindowMarker {};

void createWindows(ECS::ECSManager& ecs) noexcept {
    WITH_LOGGER_PREFIX("GUI", "GUI: ");
    for (auto entity : ecs.view<NewWindowMarker, Location>()) {
        LOG_DEBUG("Creating window @{}", entity.index());
        WinAPI::WndHandle wnd(ecs.get<Location>(entity).rect);
        ShowWindow(wnd.getHWnd(), SW_SHOW);
        ecs.insert(entity, std::move(wnd));
    }
}

void setMessageHandlers(ECS::ECSManager& ecs) noexcept {
    WITH_LOGGER_PREFIX("GUI", "GUI: ");
    using Dispatcher = std::unique_ptr<WinAPI::WindowMessageDispatcher>;
    assert(ecs.count<Dispatcher>() == 1);
    auto* dispatcherPtr = getUnique<Dispatcher>(ecs).get();
    for (auto entity : ecs.view<NewWindowMarker, WinAPI::WndHandle>()) {
        LOG_DEBUG("Set message handler @{}", entity.index());
        ecs.get<WinAPI::WndHandle>(entity).setMessageHandler(
            [entity, dispatcherPtr](WinAPI::WindowMessage message) noexcept -> LRESULT {
                return dispatcherPtr->handleMessage(entity, message);
            });
    }
}

void showWindows(ECS::ECSManager& ecs) noexcept {
    WITH_LOGGER_PREFIX("GUI", "GUI: ");
    for (auto entity : ecs.view<NewWindowMarker, WinAPI::WndHandle>()) {
        LOG_DEBUG("Show window @{}", entity.index());
        ShowWindow(ecs.get<WinAPI::WndHandle>(entity).getHWnd(), SW_SHOW);
    }
}

void cleanNewWindowMarkers(ECS::ECSManager& ecs) noexcept {
    ecs.removeAll<NewWindowMarker>();
}

void destroyWindows(ECS::ECSManager& ecs) noexcept {
    ecs.removeAll<WinAPI::WndHandle>();
}

void initGUI(ECS::ECSManager& ecs, WinAPI::QuitCallback&& quit) noexcept {
    WITH_LOGGER_PREFIX("GUI", "GUI: ");
    auto master = ecs.createEntity();
    LOG_DEBUG("Created master entity @{}", master.index());
    setupMessageDispatcher(ecs, master);
    ecs.addLoopSystem(createWindows);
    ecs.addCleanupSystem(destroyWindows);
    ecs.addLoopSystem(showWindows);
    ecs.addLoopSystem(setMessageHandlers);
    ecs.addLoopSystem(cleanNewWindowMarkers);
    ecs.addBottomLoopSystem(
        WinAPI::createMessageLoopSystem(master, std::move(quit)));
}


int main() {
    SET_LOGTERM_TRACE("");
    SET_LOGOFF("WinAPI.WndProc");

    WITH_LOGGER_PREFIX("", "App: ");
    LOG_TRACE("begin");
    {  // Scope to log on proper shutdown.
        ECS::ECSManager ecs;
        bool runFlag = true;
        auto quit = [&runFlag]() noexcept {
            LOG_DEBUG("Quit");
            runFlag = false;
        };
        initGUI(ecs, quit);

        auto window = ecs.createEntity();
        ecs.insert(window, NewWindowMarker{});
        ecs.insert(window, Location{{1100, 100, 1500, 500}});
        ecs.insert(window, EventHandlers::Close(
            [&ecs, &runFlag, quit]() noexcept {
                auto second = ecs.createEntity();
                ecs.insert(second, NewWindowMarker{});
                ecs.insert(second, Location{{1200, 200, 1400, 400}});
                ecs.insert(second, EventHandlers::Close(quit));
            }));

        while (runFlag) {
            ecs.iterate();
        }
    }  // Scope to log on proper shutdown.
    LOG_TRACE("end");
    return 0;
}

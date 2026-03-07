#include <istok/logging.hpp>
#include <istok/ecs.hpp>
#include <istok/gui.hpp>

using namespace Istok;
using namespace Istok::GUI;

int main() {
    SET_LOGTERM_TRACE("");
    SET_LOGOFF("Istok.GUI.WinAPI.WndProc");

    WITH_LOGGER_PREFIX("", "App: ");
    LOG_TRACE("begin");
    {  // Scope to log on proper shutdown.
        ECS::ECSManager ecs;
        bool runFlag = true;
        auto quit = [&runFlag]() noexcept {
            LOG_DEBUG("Quit");
            runFlag = false;
        };
        setupGUI(ecs, quit);

        auto window = ecs.createEntity();
        ecs.insert(window, CreateWindowMarker{});
        ecs.insert(window, ShowWindowMarker{});
        ecs.insert(window, WindowLocation{{1100, 100, 1500, 500}});
        ecs.insert(window, EventHandlers::Close(
            [&ecs, &runFlag, quit]() noexcept {
                auto second = ecs.createEntity();
                ecs.insert(second, CreateWindowMarker{});
                ecs.insert(second, ShowWindowMarker{});
                ecs.insert(second, WindowLocation{{1200, 200, 1400, 400}});
                ecs.insert(second, EventHandlers::Close(quit));
            }));

        ecs.addLoopSystem([](ECS::ECSManager& ecs) noexcept {
            for (auto entity : ecs.view<NewWindowMarker>()) {
                LOG_DEBUG("New window detected: {}", entity);
            }
        });

        while (runFlag) {
            ecs.iterate();
        }
    }  // Scope to log on proper shutdown.
    LOG_TRACE("end");
    return 0;
}

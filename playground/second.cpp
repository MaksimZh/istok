#include <logging.hpp>
#include <ecs.hpp>

int main() {
    SET_LOGGER(
        "",
        Istok::Logging::TerminalLogger::GetInstance(),
        Istok::Logging::Level::all);
    WITH_LOGGER_PREFIX("", "App: ");
    LOG_TRACE("begin");
    
    Istok::ECS::ECSManager ecs;
    ecs.addLoopSystem([](Istok::ECS::ECSManager& ecs) {
        LOG_TRACE("loop");
    });
    ecs.addCleanupSystem([](Istok::ECS::ECSManager& ecs) {
        LOG_TRACE("cleanup");
    });

    ecs.iterate();
    ecs.iterate();
    ecs.iterate();
    
    LOG_TRACE("end");
    return 0;
}

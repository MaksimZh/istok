#include <ecs.hpp>
#include <gui/platform/winapi.hpp>
#include <tools/queue.hpp>

#include <iostream>
#include <variant>

using namespace Istok::GUI;
using namespace Istok::ECS;

struct Renderer {
    struct Scene {
        float r;
        float g;
        float b;
        float a;
    };
};

int main() {
    EntityComponentManager ecs;
    WinAPI::Platform<Entity, Renderer> gui;
    Entity window = ecs.createEntity();
    Entity menu = ecs.createEntity();
    gui.createWindow(window, WindowParams{{200, 100, 600, 400}, "Istok"});
    gui.createWindow(menu, WindowParams{{300, 200, 400, 500}, std::nullopt});
    std::cout << "main: start" << std::endl << std::flush;
    while (true) {
        PlatformEvent<Entity> msg = gui.getMessage();
        if (std::holds_alternative<Event::PlatformHeartbeatTimeout>(msg)) {
            std::cout << "main: gui timeout" << std::endl << std::flush;
            break;
        }
        if (std::holds_alternative<Event::PlatformException>(msg)) {
            std::cout << "main: gui exception" << std::endl << std::flush;
            break;
        }
        if (std::holds_alternative<Event::PlatformShutdown>(msg)) {
            std::cout << "main: gui shutdown" << std::endl << std::flush;
            break;
        }
        if (std::holds_alternative<Event::WindowClose<Entity>>(msg)) {
            std::cout << "main: closed" << std::endl << std::flush;
            auto ent = std::get<Event::WindowClose<Entity>>(msg).id;
            gui.destroyWindow(ent);
            if (ent == window) {
                break;
            }
        }
    }
    std::cout << "main: end" << std::endl << std::flush;
    return 0;
}

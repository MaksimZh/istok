// ecs1st.cpp
#include <ecs.hpp>
#include <gui.hpp>
#include <tools/queue.hpp>

#include <iostream>
#include <variant>

using namespace Istok::GUI;
using namespace Istok::ECS;

int main() {
    EntityComponentManager ecs;
    GUIManager gui;
    Entity window = ecs.createEntity();
    Entity menu = ecs.createEntity();
    gui.newWindow(window, WindowParams{{200, 100, 600, 400}, "Istok"});
    gui.newWindow(menu, WindowParams{{300, 200, 400, 500}, std::nullopt});
    while (true) {
        AppMessage<Entity> msg = gui.getMessage();
        if (std::holds_alternative<Message::AppGUIException>(msg)) {
            std::cout << "main: gui exception" << std::endl << std::flush;
            break;
        }
        if (std::holds_alternative<Message::AppWindowClosed<Entity>>(msg)) {
            std::cout << "main: closed" << std::endl << std::flush;
            auto ent = std::get<Message::AppWindowClosed<Entity>>(msg).id;
            gui.destroyWindow(ent);
            if (ent == window) {
                break;
            }
        }
    }
    std::cout << "main: end" << std::endl << std::flush;
    return 0;
}
